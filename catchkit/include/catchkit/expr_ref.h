//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXPR_REF_H
#define CATCHKIT_EXPR_REF_H

#include "result_type.h"
#include "expression_info.h"
#include "operators.h"

#include <utility>

namespace CatchKit::Detail {

    // Holds binary expression
    template<typename LhsT, typename RhsT, Operators Op>
    struct BinaryExprRef {
        LhsT& lhs;
        RhsT& rhs;
        std::string message = {};

        [[nodiscard]] auto evaluate() const -> ResultType;
        [[nodiscard]] auto expand( ResultType result ) const -> ExpressionInfo;

    };

    // Holds a unary expression - ie just evaluates to a single value
    // Also used for the LHS of a binary expression during decomposition
    template<typename T>
    struct UnaryExprRef {
        T& value;
        std::string message = {};

        [[nodiscard]] auto evaluate() const -> ResultType;
        [[nodiscard]] auto expand( ResultType result ) const -> ExpressionInfo;

        template<Operators Op, typename RhsT>
        auto make_binary_expr( RhsT&& rhs ) noexcept { // NOSONAR (ref is used within its lifetime) NOLINT (misc-typo)
            return BinaryExprRef<T, std::remove_reference_t<RhsT>, Op>{ value, rhs };
        }

        template<typename RhsT>
        [[maybe_unused]] friend auto operator == ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( !(std::is_pointer_v<T> && !std::is_pointer_v<RhsT> && std::is_integral_v<RhsT>),
                "Comparing pointers against integrals (even 0) is not supported. Do you mean to use nullptr?");
            static_assert( requires{ lhs.value == rhs; } );
            return lhs.template make_binary_expr<Operators::Equals>( std::forward<RhsT>( rhs ) );
        }
        template<typename RhsT>
        [[maybe_unused]] friend auto operator != ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( !(std::is_pointer_v<T> && !std::is_pointer_v<RhsT> && std::is_integral_v<RhsT>),
                "Comparing pointers against integrals (even 0) is not supported. Do you mean to use nullptr?");
            static_assert( requires{ lhs.value != rhs; } );
            return lhs.template make_binary_expr<Operators::NotEqualTo>( std::forward<RhsT>( rhs ) );
        }
        template<typename RhsT>
        [[maybe_unused]] friend auto operator < ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( requires{ lhs.value < rhs; } );
            return lhs.template make_binary_expr<Operators::LessThan>( std::forward<RhsT>( rhs ) );
        }
        template<typename RhsT>
        [[maybe_unused]] friend auto operator > ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( requires{ lhs.value > rhs; } );
            return lhs.template make_binary_expr<Operators::GreaterThan>( std::forward<RhsT>( rhs ) );
        }
        template<typename RhsT>
        [[maybe_unused]] friend auto operator <= ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( requires{ lhs.value <= rhs; } );
            return lhs.template make_binary_expr<Operators::LessThanOrEqual>( std::forward<RhsT>( rhs ) );
        }
        template<typename RhsT>
        [[maybe_unused]] friend auto operator >= ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( requires{ lhs.value >= rhs; } );
            return lhs.template make_binary_expr<Operators::GreaterThanOrEqual>( std::forward<RhsT>( rhs ) );
        }
    };



} // namespace CatchKit::Detail

#endif // CATCHKIT_EXPR_REF_H
