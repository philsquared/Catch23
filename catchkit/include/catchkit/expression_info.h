//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXPRESSION_INFO_H
#define CATCHKIT_EXPRESSION_INFO_H

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

        std::string_view op;
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
    [[nodiscard]] constexpr static auto parse(std::format_parse_context const& ctx) { return ctx.begin(); }
    static std::format_context::iterator format(CatchKit::ExpressionInfo const& expr, std::format_context& ctx);
};

#endif // CATCHKIT_EXPRESSION_INFO_H
