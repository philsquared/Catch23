//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_CHECKER_H
#define CATCHKIT_CHECKER_H

#include "catchkit_result_handler.h"
#include "catchkit_assertion_context.h"
#include "catchkit_expr_ref.h"
#include "catchkit_internal_warnings.h"
#include "catchkit_operator_to_string.h"
#include "catchkit_decomposer.h"
#include "catchkit_exceptions.h"
#include "catchkit_stringify.h"

#include <utility>

namespace CatchKit::Detail
{
    struct Asserter;

    struct Checker {
        ResultHandler& result_handler;
        ResultDisposition result_disposition;
        bool should_decompose = true;

        auto operator()(std::string_view message = {}, std::source_location assertion_location = std::source_location::current()) -> Asserter;
        auto operator()(AssertionContext&& context) -> Asserter;
    };

    struct Asserter {
        Checker& checker;

        ~Asserter() noexcept(false) {
            checker.result_handler.on_assertion_end(); // This may throw to cancel the test
        }
        void handle_unexpected_exceptions(std::invocable<Asserter&> auto const& expr_call) {
            try {
                expr_call(*this);
            }
            catch(...) {
                checker.result_handler.on_assertion_result( ResultType::UnexpectedException, {}, get_exception_message(std::current_exception()) );
            }
        }

        template<typename T>
        void simple_assert(auto const&, T&&) noexcept {
            static_assert(std::is_convertible_v<T, std::string_view>, "Only matchers or strings can follow the comma operator");
        }
        void simple_assert(std::nullptr_t, std::string_view message = {}) noexcept {
            simple_assert(false, message);
        }
        void simple_assert(auto const& result, std::string_view message = {}) noexcept {
            bool is_failure = !result;
            if( checker.result_handler.report_on == ReportOn::AllResults || is_failure ) {}
                checker.result_handler.on_assertion_result(is_failure ? ResultType::ExpressionFailed : ResultType::Pass, {}, message);
        }
        void accept_expr(auto& expr) noexcept; // Implemented after the definitions of the Expr Ref types

        // To kick off an expression decomposition
        template<typename LhsT>
        [[maybe_unused]] friend constexpr auto operator << ( Asserter& asserter, LhsT&& lhs ) noexcept {
            return UnaryExprRef{ lhs, &asserter };
        }
        template<typename LhsT>
        [[maybe_unused]] friend constexpr auto operator << ( Asserter&& asserter, LhsT&& lhs ) noexcept {
            return UnaryExprRef{ lhs, &asserter };
        }
    };

    // --------------

    template<typename T>
    UnaryExprRef<T>::~UnaryExprRef() {
        if( asserter )
            asserter->accept_expr(*this);
    }

    template<typename T>
    auto UnaryExprRef<T>::evaluate() -> ResultType {
        if constexpr( requires (T v){ { !v } -> std::same_as<bool>; }) {
            CATCHKIT_WARNINGS_SUPPRESS_START
            CATCHKIT_WARNINGS_SUPPRESS_ADDRESS
            CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION
            return !value ? ResultType::ExpressionFailed : ResultType::Pass;
            CATCHKIT_WARNINGS_SUPPRESS_END
        }
        else if constexpr( std::is_null_pointer_v<T> ) {
            // Special case for GCC
            return ResultType::ExpressionFailed;
        }
        else {
            // Have to do this at runtime because we can get here from the destructor of a UnaryExpr,
            // even if it doesn't happen at runtime because it's actually a binary expresion
            throw std::logic_error("Attempt to use a value that cannot convert to bool in boolean context");
        }
    }
    template<typename T>
    auto UnaryExprRef<T>::expand(ResultType) -> ExpressionInfo {
        return ExpressionInfo{ {std::string(stringify(value))}, {}, Operators::None, {} };
    }

    template<typename LhsT, typename RhsT, Operators Op>
    BinaryExprRef<LhsT, RhsT, Op>::~BinaryExprRef() {
        if( asserter )
            asserter->accept_expr(*this);
    }

    CATCHKIT_WARNINGS_SUPPRESS_START CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH
    template<typename LhsT, typename RhsT, Operators Op>
    auto eval_expr(BinaryExprRef<LhsT, RhsT, Op>& expr) {
        using enum Operators;
        if constexpr( Op == Equals )                    return expr.lhs == expr.rhs;
        else if constexpr( Op == NotEqualTo )           return expr.lhs != expr.rhs;
        else if constexpr( Op == GreaterThan )          return expr.lhs >  expr.rhs;
        else if constexpr( Op == LessThan )             return expr.lhs <  expr.rhs;
        else if constexpr( Op == GreaterThanOrEqual )   return expr.lhs >= expr.rhs;
        else if constexpr( Op == LessThanOrEqual )      return expr.lhs <= expr.rhs;
        else {
            // Note that while None is a valid enum value, we should never use it on a path that leads here
            static_assert( false, "Operator not implemented" );
        }
        std::unreachable();
    }
    CATCHKIT_WARNINGS_SUPPRESS_END

    template<typename LhsT, typename RhsT, Operators Op>
    auto BinaryExprRef<LhsT, RhsT, Op>::evaluate() -> ResultType {
        return static_cast<bool>( eval_expr(*this) ) ? ResultType::Pass : ResultType::ExpressionFailed;
    }
    template<typename LhsT, typename RhsT, Operators Op>
    auto BinaryExprRef<LhsT, RhsT, Op>::expand(ResultType) -> ExpressionInfo {
        return ExpressionInfo{
            std::string( stringify(lhs) ),
            std::string( stringify(rhs) ),
            Op,
            operator_to_string<Op>() };
    }

    template<typename ArgT, typename MatcherT>
    MatchExprRef<ArgT, MatcherT>::~MatchExprRef() {
        if( asserter )
            asserter->accept_expr(*this);
    }

    // -------

    void Asserter::accept_expr( auto& expr ) noexcept {
        auto result = expr.evaluate();

        if( checker.result_handler.report_on == ReportOn::AllResults || result != ResultType::Pass ) {
            checker.result_handler.on_assertion_result( result, expr.expand(result), expr.message );
        }
        else {
            checker.result_handler.on_assertion_result( result, {}, expr.message );
        }
    }

} // namespace CatchKit::Detail

namespace CatchKit
{
    using Detail::Checker;

} //namespace CatchKit

// These global instances are used if not using the ones passed in to a function locally
extern constinit CatchKit::Checker check, require;


#define CATCHKIT_ASSERT_INTERNAL(macro_name, checker, ...) \
if(checker.should_decompose) { \
    CATCHKIT_WARNINGS_SUPPRESS_START CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON \
    checker(CatchKit::AssertionContext(macro_name, #__VA_ARGS__)).handle_unexpected_exceptions([&](CatchKit::Detail::Asserter& asserter){ asserter << __VA_ARGS__; }); \
    CATCHKIT_WARNINGS_SUPPRESS_END \
} else checker(CatchKit::AssertionContext(macro_name, #__VA_ARGS__)).simple_assert(__VA_ARGS__)


#define CATCHKIT_ASSERT_THAT_INTERNAL(macro_name, checker, arg, match_expr) \
do { using namespace CatchKit::Matchers; \
    checker(CatchKit::AssertionContext(macro_name, #arg ", " #match_expr)) << [&]{ return arg; }, match_expr; \
} while( false )


#define CHECK(...) CATCHKIT_ASSERT_INTERNAL( "CHECK", check, __VA_ARGS__ )
#define REQUIRE(...) CATCHKIT_ASSERT_INTERNAL( "REQUIRE", require, __VA_ARGS__ )

#define CHECK_THAT( arg, matcher ) CATCHKIT_ASSERT_THAT_INTERNAL( "CHECK_THAT", check, arg, matcher )
#define REQUIRE_THAT( arg, matcher ) CATCHKIT_ASSERT_THAT_INTERNAL( "REQUIRE_THAT", require, arg, matcher )

// !TBD: These should have a dedicated internal macro (in Catch2 it was INTERNAL_CATCH_MSG)
#define PASS(...) CATCHKIT_ASSERT_INTERNAL( "PASS", check, true __VA_OPT__(,) __VA_ARGS__ )
#define FAIL(...) CATCHKIT_ASSERT_INTERNAL( "FAIL", require, false __VA_OPT__(,) __VA_ARGS__ )


#endif // CATCHKIT_CHECKER_H
