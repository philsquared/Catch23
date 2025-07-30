//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXPR_REF_H
#define CATCHKIT_EXPR_REF_H

#include "result_type.h"
#include "expression_info.h"

#include <vector>
#include <cassert>
#include <utility>

namespace CatchKit::Detail {

    struct Asserter;

    // Holds a unary expression - ie just evaluates to a single value
    // Also used for the LHS of a binary expression during decomposition
    template<typename T>
    struct UnaryExprRef {
        T& value;
        Asserter* asserter = nullptr;
        std::string message = {};

        auto evaluate() -> ResultType;
        auto expand( ResultType result ) -> ExpressionInfo;

        ~UnaryExprRef();
    };

    // Holds binary expression
    template<typename LhsT, typename RhsT, Operators Op>
    struct BinaryExprRef {
        LhsT& lhs;
        RhsT& rhs;
        Asserter* asserter = nullptr;
        std::string message = {};

        auto evaluate() -> ResultType;
        auto expand( ResultType result ) -> ExpressionInfo;

        ~BinaryExprRef();
    };

    struct MatchResult;

    template<typename ArgT, typename MatcherT>
    struct MatchExprRef {
        ArgT& arg;
        MatcherT const& matcher;
        Asserter* asserter = nullptr;
        std::string message = {};

        auto evaluate() -> MatchResult;
        auto expand( MatchResult const& result ) -> ExpressionInfo;

        ~MatchExprRef();
    };

    struct SubExpression {
        bool result;
        void const* matcher_address;
    };

    // Holds the result of a match
    struct MatchResult {
        bool result;
        void const* matcher_address = nullptr;
        std::vector<SubExpression> child_results; // TBD: only include this in composite matcher results?
        explicit(false) MatchResult(bool result, void const* matcher_address = nullptr) : result(result), matcher_address(matcher_address) {}
        explicit operator bool() const { return result; }

        auto set_address(void const* address) -> MatchResult& {
            assert(matcher_address == nullptr || matcher_address == address);
            matcher_address = address;
            return *this;
        }
        auto add_children_from(MatchResult const& other) -> MatchResult& {
            child_results.reserve( child_results.size() + other.child_results.size() );
            std::ranges::copy( other.child_results, std::back_inserter( child_results ) );
            return *this;
        }
        auto make_child_of(void const* new_matcher_address) -> MatchResult& {
            child_results.emplace_back( result, std::exchange( matcher_address, new_matcher_address ) );
            return *this;
        }
    };

} // namespace CatchKit::Detail

#endif // CATCHKIT_EXPR_REF_H
