//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/expression_info.h"

#include <cassert>
#include <utility>

std::format_context::iterator std::formatter<CatchKit::ExpressionInfo>::format(CatchKit::ExpressionInfo const& expr, std::format_context& ctx) {
    using namespace CatchKit;

    return std::visit(
        [&]<typename T>(T const& expr_inner) -> std::format_context::iterator {
            if constexpr(std::is_same_v<T, UnaryExpressionInfo>)
                return std::format_to( ctx.out(), "{}", expr_inner.value );
            else if constexpr(std::is_same_v<T, BinaryExpressionInfo>)
                return std::format_to( ctx.out(), "{} {} {}", expr_inner.lhs, expr_inner.op, expr_inner.rhs );
            else if constexpr(std::is_same_v<T, MatchExpressionInfo>)
                return std::format_to( ctx.out(), "{} {}", expr_inner.candidate_value, expr_inner.matcher );
            else if constexpr(std::is_same_v<T, ExceptionExpressionInfo>)
                return std::format_to( ctx.out(), "{}", expr_inner.exception_message );
            else if constexpr(std::is_same_v<T, ExpectationExpressionInfo>) {
                assert(false); // Not yet implemented
                return std::format_to( ctx.out(), "{}", "{not implemented}" );
            }
            else {
                static_assert(std::is_same_v<T, monostate>);
                std::unreachable();
            }
    }, expr);
}
