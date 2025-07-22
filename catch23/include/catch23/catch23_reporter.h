//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_REPORTER_H
#define CATCH23_REPORTER_H

#include "catch23_test_info.h"

#include "catchkit/catchkit_expression_info.h"
#include "catchkit/catchkit_result_type.h"
#include "catchkit/catchkit_assertion_context.h"

#include <optional>

namespace CatchKit {

    struct AssertionInfo {
        ResultType result;
        std::optional<ExpressionInfo> expression_info;
        std::string message;

        [[nodiscard]] auto passed() const { return result == ResultType::Pass; }
    };

    struct Reporter {
        virtual ~Reporter() = default;

        virtual void on_test_start( TestInfo const& test_info ) = 0;
        virtual void on_test_end( TestInfo const& test_info ) = 0;

        virtual void on_assertion_start( AssertionContext const& context ) = 0;
        virtual void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) = 0;
    };

} // namespace CatchKit

#endif //CATCH23_REPORTER_H
