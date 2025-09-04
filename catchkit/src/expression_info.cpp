//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/expression_info.h"

#include <cassert>

std::format_context::iterator std::formatter<CatchKit::ExpressionInfo>::format(const CatchKit::ExpressionInfo& expr, std::format_context& ctx) const {
    using namespace CatchKit;

    return std::visit(
        [&]<typename T>(T const& expr) -> std::format_context::iterator {
            if constexpr(std::is_same_v<T, UnaryExpressionInfo>)
                return std::format_to( ctx.out(), "{}", expr.value );
            if constexpr(std::is_same_v<T, BinaryExpressionInfo>)
                return std::format_to( ctx.out(), "{} {} {}", expr.lhs, expr.op_str, expr.rhs );
            if constexpr(std::is_same_v<T, MatchExpressionInfo>)
                return std::format_to( ctx.out(), "{} {}", expr.candidate_value, expr.matcher );
            if constexpr(std::is_same_v<T, ExceptionExpressionInfo>)
                return std::format_to( ctx.out(), "{}", expr.exception_message );
            assert(false);
    }, expr);
}
