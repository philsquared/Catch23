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


    // !TBD: This could do with a rethink.
    // It has evolved to be several different things, depending on expression type.
    // ExpressionType has now been added, which is a start.
    // Current thinking:
    // - Expand ExpressionType to include most of the things in ResultType, like exceptions.
    // - Moved ExpressionType into AssertionInfo and have ResultType just be a pass/ fail
    // - Have different ExpressionInfo types for different types of expression and hold them in a variant
    struct ExpressionInfo {
        std::string lhs;
        std::string rhs;

        Operators op;
        std::string_view op_str;

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
