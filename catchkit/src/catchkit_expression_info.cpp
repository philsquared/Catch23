//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/catchkit_expression_info.h"

std::format_context::iterator std::formatter<CatchKit::Detail::ExpressionInfo>::format(const CatchKit::Detail::ExpressionInfo& expr, std::format_context& ctx) const {
    if( expr.op == CatchKit::Detail::Operators::None )
        return std::format_to(ctx.out(), "{}", expr.lhs);
    else
        return std::format_to(ctx.out(), "{} {} {}", expr.lhs, expr.op_str, expr.rhs);
}
