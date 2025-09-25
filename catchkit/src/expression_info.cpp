//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/expression_info.h"

#include <cassert>

std::format_context::iterator std::formatter<CatchKit::ExpressionInfo>::format(CatchKit::ExpressionInfo const& expr, std::format_context& ctx) {
    using namespace CatchKit;

    return std::visit(
        [&]<typename T>(T const& expr_inner) -> std::format_context::iterator {
            if constexpr(std::is_same_v<T, UnaryExpressionInfo>)
                return std::format_to( ctx.out(), "{}", expr_inner.value );
            if constexpr(std::is_same_v<T, BinaryExpressionInfo>)
                return std::format_to( ctx.out(), "{} {} {}", expr_inner.lhs, expr_inner.op, expr_inner.rhs );
            if constexpr(std::is_same_v<T, MatchExpressionInfo>)
                return std::format_to( ctx.out(), "{} {}", expr_inner.candidate_value, expr_inner.matcher );
            if constexpr(std::is_same_v<T, ExceptionExpressionInfo>)
                return std::format_to( ctx.out(), "{}", expr_inner.exception_message );
            assert(false);
    }, expr);
}
