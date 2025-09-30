//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXPR_REF_H
#define CATCHKIT_EXPR_REF_H

#include "result_type.h"
#include "expression_info.h"
#include "operators.h"

#include <vector>
#include <cassert>
#include <utility>


namespace CatchKit::Detail {

    struct Asserter;

    // Holds binary expression
    template<typename LhsT, typename RhsT, Operators Op>
    struct BinaryExprRef {
        LhsT& lhs;
        RhsT& rhs;
        Asserter* asserter = nullptr;
        std::string message = {};

        [[nodiscard]] auto evaluate() const -> ResultType;
        [[nodiscard]] auto expand( ResultType result ) const -> ExpressionInfo;

        ~BinaryExprRef();

        [[maybe_unused]] auto&& operator, ( std::string_view new_message ) noexcept {
            this->message = new_message;
            return *this;
        }

    };

    // Holds a unary expression - ie just evaluates to a single value
    // Also used for the LHS of a binary expression during decomposition
    template<typename T>
    struct UnaryExprRef {
        T& value;
        Asserter* asserter = nullptr;
        std::string message = {};

        [[nodiscard]] auto evaluate() const -> ResultType;
        [[nodiscard]] auto expand( ResultType result ) const -> ExpressionInfo;

        ~UnaryExprRef();

        template<Operators Op, typename RhsT>
        auto make_binary_expr( RhsT&& rhs ) noexcept { // NOSONAR (ref is used within its lifetime) NOLINT (misc-typo)
            return BinaryExprRef<T, std::remove_reference_t<RhsT>, Op>{ value, rhs, std::exchange(asserter, nullptr) };
        }

        template<typename RhsT>
        [[maybe_unused]] friend auto operator == ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
            static_assert( requires{ lhs.value == rhs; } );
            return lhs.template make_binary_expr<Operators::Equals>( std::forward<RhsT>( rhs ) );
        }
        template<typename RhsT>
        [[maybe_unused]] friend auto operator != ( UnaryExprRef lhs, RhsT&& rhs ) noexcept {
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

        [[maybe_unused]] auto&& operator, ( std::string_view new_message ) noexcept {
            this->message = new_message;
            return *this;
        }

    };



} // namespace CatchKit::Detail

#endif // CATCHKIT_EXPR_REF_H
