//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXPR_REF_H
#define CATCHKIT_EXPR_REF_H

#include "catchkit_result_type.h"
#include "catchkit_expression_info.h"

#include <string_view>

namespace CatchKit::Detail {

    struct Checker;

    // Holds a unary expression - ie just evaluates to a single value
    // Also used for the LHS of a binary expression during decomposition
    template<typename T>
    struct UnaryExprRef {
        T& value;
        Checker* checker = nullptr;
        std::string_view message = {};

        auto evaluate() -> ResultType;
        auto expand( ResultType result ) -> ExpressionInfo;

        ~UnaryExprRef();
    };

    // Holds binary expression
    template<typename LhsT, typename RhsT, Operators Op>
    struct BinaryExprRef {
        LhsT& lhs;
        RhsT& rhs;
        Checker* checker = nullptr;
        std::string_view message = {};

        auto evaluate() -> ResultType;
        auto expand( ResultType result ) -> ExpressionInfo;

        ~BinaryExprRef();
    };

    template<typename ArgT, typename MatcherT>
    struct MatchExprRef {
        ArgT& arg;
        MatcherT const& matcher;
        Checker* checker = nullptr;
        std::string_view message = {};

        auto evaluate() -> ResultType;
        auto expand( ResultType result ) -> ExpressionInfo;

        ~MatchExprRef();
    };

    // Holds the result of a match
    struct MatchResult {
        bool result;
        explicit(false) MatchResult(bool result) : result(result) {}
        explicit operator bool() const { return result; }
    };

} // namespace CatchKit::Detail

#endif //CATCHKIT_EXPR_REF_H
