//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_CHECKER_H
#define CATCHKIT_CHECKER_H

#include "result_handler.h"
#include "assertion_context.h"
#include "expr_ref.h"
#include "internal_warnings.h"
#include "operator_to_string.h"
#include "exceptions.h"
#include "stringify.h"
#include "variable_capture.h"

#include <utility>

namespace CatchKit::Detail
{
    struct Asserter;

    struct Checker {
        ResultHandler* result_handler;
        ResultDisposition result_disposition;
        bool should_decompose = true;

        auto operator()(std::string_view message = {}, std::source_location assertion_location = std::source_location::current()) -> Asserter;
        auto operator()(AssertionContext const& context) -> Asserter;
    };

    struct Asserter {
        Checker& checker;

        ~Asserter() noexcept(false);

        void handle_unexpected_exceptions(std::invocable<Asserter&> auto const& expr_call) {
            try {
                expr_call(*this);
            }
            catch(...) {
                report_current_exception();
            }
        }
        void handle_unexpected_exceptions(std::invocable auto const& expr_call) {
            try {
                expr_call();
            }
            catch(...) {
                report_current_exception();
            }
        }

        template<typename T>
        void simple_assert(auto const&, T&&) const noexcept {
            static_assert(std::is_convertible_v<T, std::string_view>, "Only matchers or strings can follow the comma operator");
        }
        void simple_assert(std::nullptr_t, std::string_view message = {}) const noexcept {
            simple_assert(false, message);
        }
        void simple_assert(auto const& result, std::string_view message = {}) const noexcept {
            if( checker.result_handler->on_assertion_result(!result ? ResultType::Failed : ResultType::Passed) == ResultDetailNeeded::Yes )
                checker.result_handler->on_assertion_result_detail(std::monostate(), message);
        }
        void accept_expr(auto& expr) noexcept; // Implemented after the definitions of the Expr Ref types

        template<typename ArgT, typename MatcherT> // !TBD Matcher concept
        constexpr auto that( ArgT&& arg, MatcherT const& matcher ) noexcept {
            // !TBD: Should we use only this path, or keep the decomp operator path, too?
            return MatchExprRef{ arg, matcher, this };
        }

        // To kick off an expression decomposition
        [[maybe_unused]] friend constexpr auto operator << ( Asserter& asserter, auto&& lhs ) noexcept {
            return UnaryExprRef{ lhs, &asserter };
        }
        [[maybe_unused]] friend constexpr auto operator << ( Asserter&& asserter, auto&& lhs ) noexcept {
            return UnaryExprRef{ lhs, &asserter };
        }
    private:
        void report_current_exception() const;
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
            return !value ? ResultType::Failed : ResultType::Passed;
            CATCHKIT_WARNINGS_SUPPRESS_END
        }
        else if constexpr( std::is_null_pointer_v<T> ) {
            // Special case for GCC
            return ResultType::Failed;
        }
        else {
            // Have to do this at runtime because we can get here from the destructor of a UnaryExpr,
            // even if it doesn't happen at runtime because it's actually a binary expresion
            throw std::logic_error("Attempt to use a value that cannot convert to bool in boolean context");
        }
    }
    template<typename T>
    auto UnaryExprRef<T>::expand(ResultType) -> ExpressionInfo {
        return UnaryExpressionInfo{ stringify(value) };
    }

    template<typename LhsT, typename RhsT, Operators Op>
    BinaryExprRef<LhsT, RhsT, Op>::~BinaryExprRef() {
        if( asserter )
            asserter->accept_expr(*this);
    }

    CATCHKIT_WARNINGS_SUPPRESS_START
    CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH
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
        return static_cast<bool>( eval_expr(*this) ) ? ResultType::Passed : ResultType::Failed;
    }
    template<typename LhsT, typename RhsT, Operators Op>
    auto BinaryExprRef<LhsT, RhsT, Op>::expand(ResultType) -> ExpressionInfo {
        return BinaryExpressionInfo{
            std::string( stringify(lhs) ),
            std::string( stringify(rhs) ),
            operator_to_string<Op>() };
    }

    template<typename ArgT, typename MatcherT>
    MatchExprRef<ArgT, MatcherT>::~MatchExprRef() {
        if( asserter )
            asserter->accept_expr(*this);
    }

    inline auto to_result_type( MatchResult const& result ) -> ResultType { return result ? ResultType::Passed : ResultType::Failed; }

    // -------

    inline auto to_result_type( ResultType result ) -> ResultType { return result; }

    void Asserter::accept_expr( auto& expr ) noexcept {
        auto raw_result = expr.evaluate();
        if( checker.result_handler->on_assertion_result( to_result_type( raw_result ) ) == ResultDetailNeeded::Yes )
            checker.result_handler->on_assertion_result_detail( expr.expand( raw_result ), expr.message );
    }

} // namespace CatchKit::Detail

namespace CatchKit
{
    using Detail::Checker;

} //namespace CatchKit

// These global instances are used if not using the ones passed in to a function locally
extern constinit CatchKit::Checker check; // NOLINT
extern constinit CatchKit::Checker require; // NOLINT


#define CATCHKIT_ASSERT_INTERNAL(macro_name, checker, ...) \
    if( checker.should_decompose ) \
        checker( CatchKit::AssertionContext(macro_name, #__VA_ARGS__) ).handle_unexpected_exceptions([&](CatchKit::Detail::Asserter& asserter){ \
            CATCHKIT_WARNINGS_SUPPRESS_START \
            CATCHKIT_WARNINGS_SUPPRESS_UNUSED_COMPARISON \
            asserter << __VA_ARGS__; \
            CATCHKIT_WARNINGS_SUPPRESS_END \
        }); \
    else checker(CatchKit::AssertionContext(macro_name, #__VA_ARGS__)).simple_assert(__VA_ARGS__)


#define CATCHKIT_ASSERT_THAT_INTERNAL(macro_name, checker, arg, match_expr) \
    do { using namespace CatchKit::Matchers; \
        checker(CatchKit::AssertionContext(macro_name, #arg ", " #match_expr)).that( [&]{ return arg; }, match_expr ); \
    } while( false )


#define CHECK(...) CATCHKIT_ASSERT_INTERNAL( "CHECK", check, __VA_ARGS__ )
#define REQUIRE(...) CATCHKIT_ASSERT_INTERNAL( "REQUIRE", require, __VA_ARGS__ )

#define CHECK_THAT( arg, matcher ) CATCHKIT_ASSERT_THAT_INTERNAL( "CHECK_THAT", check, arg, matcher )
#define REQUIRE_THAT( arg, matcher ) CATCHKIT_ASSERT_THAT_INTERNAL( "REQUIRE_THAT", require, arg, matcher )

#define REQUIRE_STATIC(...) static_assert(__VA_ARGS__)

// !TBD: These should have a dedicated internal macro (in Catch2 it was INTERNAL_CATCH_MSG)
#define PASS(...) CATCHKIT_ASSERT_INTERNAL( "PASS", check, true __VA_OPT__(,) __VA_ARGS__ )
#define FAIL(...) CATCHKIT_ASSERT_INTERNAL( "FAIL", require, false __VA_OPT__(,) __VA_ARGS__ )


#endif // CATCHKIT_CHECKER_H
