//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXPRESSION_INFO_H
#define CATCHKIT_EXPRESSION_INFO_H

#include "operators.h"

#include <string_view>
#include <vector>
#include <string>
#include <format>

namespace CatchKit::Detail {

    struct SubExpressionInfo {
        std::string description;
        bool result;
    };

    enum class ExpressionType {
        Unary,
        Binary,
        Match,
        Expectation
    };

    struct ExpressionInfo
    {
        std::string lhs;
        std::string rhs;

        Operators op;
        std::string_view op_str;

        ExpressionType expression_type;
        std::vector<SubExpressionInfo> sub_expressions = {};
    };

} // namespace CatchKit::Detail

namespace CatchKit {

    using Detail::ExpressionInfo;

}

template<>
struct std::formatter<CatchKit::Detail::ExpressionInfo> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    std::format_context::iterator format(const CatchKit::Detail::ExpressionInfo& expr, std::format_context& ctx) const;
};

#endif // CATCHKIT_EXPRESSION_INFO_H
