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
#include <variant>

namespace CatchKit {

    struct SubExpressionInfo {
        std::string description;
        bool result;
    };


    struct UnaryExpressionInfo {
        std::string value;
    };
    struct BinaryExpressionInfo {
        std::string lhs;
        std::string rhs;

        Detail::Operators op;
        std::string_view op_str;
    };
    struct MatchExpressionInfo {
        std::string candidate_value;
        std::string matcher;

        std::vector<SubExpressionInfo> sub_expressions;
    };
    struct ExceptionExpressionInfo {
        std::string exception_message;
        enum class Type { Expected, Unexpected, Missing };
        Type type;
    };
    struct ExpectationExpressionInfo {
        // !TBD
    };

    using ExpressionInfo = std::variant<
        std::monostate,
        UnaryExpressionInfo,
        BinaryExpressionInfo,
        MatchExpressionInfo,
        ExceptionExpressionInfo,
        ExpectationExpressionInfo>;

} // namespace CatchKit

template<>
struct std::formatter<CatchKit::ExpressionInfo> {
    constexpr auto parse(std::format_parse_context& ctx) { return ctx.begin(); }
    std::format_context::iterator format(const CatchKit::ExpressionInfo& expr, std::format_context& ctx) const;
};

#endif // CATCHKIT_EXPRESSION_INFO_H
