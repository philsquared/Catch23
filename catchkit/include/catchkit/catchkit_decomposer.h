//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_DECOMPOSER_H
#define CATCHKIT_DECOMPOSER_H

#include "catchkit_expr_ref.h"

#include <utility>

namespace CatchKit::Detail {

    // To decompose a comparison with another value
    template<typename LhsT, typename RhsT> requires requires( LhsT lhs, RhsT rhs ) { lhs == rhs; }
    [[maybe_unused]] constexpr auto operator == ( UnaryExprRef<LhsT>&& lhs, RhsT&& rhs ) noexcept {
        return BinaryExprRef<LhsT, std::remove_reference_t<RhsT>, Operators::Equals>{ lhs.value, rhs, std::exchange(lhs.checker, nullptr) };
    }
    template<typename LhsT, typename RhsT> requires requires( LhsT lhs, RhsT rhs ) { lhs != rhs; }
    [[maybe_unused]] constexpr auto operator != ( UnaryExprRef<LhsT>&& lhs, RhsT&& rhs ) noexcept {
        return BinaryExprRef<LhsT, std::remove_reference_t<RhsT>, Operators::NotEqualTo>{ lhs.value, rhs, std::exchange(lhs.checker, nullptr) };
    }
    template<typename LhsT, typename RhsT> requires requires( LhsT lhs, RhsT rhs ) { lhs > rhs; }
    [[maybe_unused]] constexpr auto operator > ( UnaryExprRef<LhsT>&& lhs, RhsT&& rhs ) noexcept {
        return BinaryExprRef<LhsT, std::remove_reference_t<RhsT>, Operators::GreaterThan>{ lhs.value, rhs, std::exchange(lhs.checker, nullptr) };
    }
    template<typename LhsT, typename RhsT> requires requires( LhsT lhs, RhsT rhs ) { lhs < rhs; }
    [[maybe_unused]] constexpr auto operator < ( UnaryExprRef<LhsT>&& lhs, RhsT&& rhs ) noexcept {
        return BinaryExprRef<LhsT, std::remove_reference_t<RhsT>, Operators::LessThan>{ lhs.value, rhs, std::exchange(lhs.checker, nullptr) };
    }
    template<typename LhsT, typename RhsT> requires requires( LhsT lhs, RhsT rhs ) { lhs >= rhs; }
    [[maybe_unused]] constexpr auto operator >= ( UnaryExprRef<LhsT>&& lhs, RhsT&& rhs ) noexcept {
        return BinaryExprRef<LhsT, std::remove_reference_t<RhsT>, Operators::GreaterThanOrEqual>{ lhs.value, rhs, std::exchange(lhs.checker, nullptr) };
    }
    template<typename LhsT, typename RhsT> requires requires( LhsT lhs, RhsT rhs ) { lhs <= rhs; }
    [[maybe_unused]] constexpr auto operator <= ( UnaryExprRef<LhsT>&& lhs, RhsT&& rhs ) noexcept {
        return BinaryExprRef<LhsT, std::remove_reference_t<RhsT>, Operators::LessThanOrEqual>{ lhs.value, rhs, std::exchange(lhs.checker, nullptr) };
    }

    template<typename ArgT>
    [[maybe_unused]] constexpr auto operator, ( UnaryExprRef<ArgT>&& value_ref, std::string_view message ) noexcept {
        value_ref.message = message;
        return value_ref;
    }
    template<typename LhsT, typename RhsT, Operators Op>
    [[maybe_unused]] constexpr auto operator, ( BinaryExprRef<LhsT, RhsT, Op>&& expr_ref, std::string_view message ) noexcept {
        expr_ref.message = message;
        return expr_ref;
    }

} // namespace CatchKit::Detail

#endif // CATCHKIT_DECOMPOSER_H
