//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_CHECKER_H
#define CATCHKIT_CHECKER_H

#include "result_handler.h"
#include "assertion_context.h" // NOLINT (misc-include-cleaner)
#include "expr_ref.h"
#include "internal_warnings.h"
#include "operator_to_string.h"
#include "exceptions.h"
#include "stringify.h"
#include "variable_capture.h" // NOLINT (misc-include-cleaner)
#include "macros.h" // NOLINT (misc-include-cleaner)

#include <utility>
#include <cassert>
#include <sstream>

namespace CatchKit::Detail
{
    class Asserter;
    struct MatchResult;

    struct Checker {
        ResultHandler* result_handler = nullptr;
        ResultDisposition result_disposition = ResultDisposition::Abort;
        bool should_decompose = true;
        std::optional<std::ostringstream> message_stream = {};

        auto operator()(AssertionContext const& context) -> Asserter;
    };

    inline auto to_result_type( ResultType result ) -> ResultType { return result; }
    auto to_result_type( MatchResult const& result ) -> ResultType; // Implemented in internal_matchers.h

    class Asserter {
        Checker& checker;

        void report_current_exception() const;
        std::optional<ExpressionInfo> expression_info;
    public:
        explicit Asserter( Checker& checker );
        ~Asserter() noexcept(false); // NOSONAR NOLINT (misc-typo)

        auto& handle_unexpected_exceptions(std::invocable<Asserter&> auto const& expr_call) {
            try {
                expr_call(*this);
            }
            catch(...) {
                report_current_exception();
            }
            return *this;
        }

        void simple_assert(std::nullptr_t) noexcept {
            simple_assert(false);
        }
        void simple_assert(auto const& result) noexcept {
            if( checker.result_handler->on_assertion_result(!result ? ResultType::Failed : ResultType::Passed) == ResultDetailNeeded::Yes ) {
                expression_info = std::monostate();
            }
        }
        void accept_expr(auto&& expr) noexcept {  // NOSONAR NOLINT (misc-typo)
            auto raw_result = expr.evaluate();
            if( checker.result_handler->on_assertion_result( to_result_type( raw_result ) ) == ResultDetailNeeded::Yes ) {
                expression_info = expr.expand( raw_result );
            }
        }

        template<typename ArgT, typename MatcherT>
        constexpr void that( ArgT&& arg, MatcherT&& matcher ) noexcept;

        [[maybe_unused]] friend auto& operator << ( Asserter& asserter, auto&& message ) {
            if( asserter.expression_info ) {
                if( !asserter.checker.message_stream )
                    asserter.checker.message_stream.emplace();
                *asserter.checker.message_stream << message;
            }
            return asserter;
        }

        // To kick off an expression decomposition
        [[maybe_unused]] friend constexpr auto operator <=> ( Asserter const&, auto&& value ) noexcept { // NOSONAR NOLINT (misc-typo)
            return UnaryExprRef{ value };
        }
    };

    // --------------

    template<typename T>
    auto UnaryExprRef<T>::evaluate() const -> ResultType {
        using enum ResultType;
        if constexpr( requires (T v){ { !v } -> std::same_as<bool>; }) {
            CATCHKIT_WARNINGS_SUPPRESS_START
            CATCHKIT_WARNINGS_SUPPRESS_ADDRESS
            CATCHKIT_WARNINGS_SUPPRESS_NULL_CONVERSION
            return !value ? Failed : Passed;
            CATCHKIT_WARNINGS_SUPPRESS_END
        }
        else {
            // Special case for GCC
            static_assert( std::is_null_pointer_v<T> );
            return Failed;
        }
    }
    template<typename T>
    auto UnaryExprRef<T>::expand(ResultType) const -> ExpressionInfo {
        return UnaryExpressionInfo{ stringify(value) };
    }

    CATCHKIT_WARNINGS_SUPPRESS_START
    CATCHKIT_WARNINGS_SUPPRESS_SIGN_MISMATCH
    template<typename LhsT, typename RhsT, Operators Op>
    auto eval_expr(BinaryExprRef<LhsT, RhsT, Op> const& expr) {
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
    auto BinaryExprRef<LhsT, RhsT, Op>::evaluate() const -> ResultType {
        return static_cast<bool>( eval_expr(*this) ) ? ResultType::Passed : ResultType::Failed;
    }
    template<typename LhsT, typename RhsT, Operators Op>
    auto BinaryExprRef<LhsT, RhsT, Op>::expand(ResultType) const -> ExpressionInfo {
        return BinaryExpressionInfo{
            std::string( stringify(lhs) ),
            std::string( stringify(rhs) ),
            operator_to_string<Op>() };
    }

} // namespace CatchKit::Detail

namespace CatchKit
{
    using Detail::Checker;

} // namespace CatchKit

// These global instances are used if not using the ones passed in to a function locally
extern constinit CatchKit::Checker check; // NOSONAR NOLINT (misc-typo)
extern constinit CatchKit::Checker require; // NOSONAR NOLINT (misc-typo)

#endif // CATCHKIT_CHECKER_H
