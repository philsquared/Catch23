//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_REPORTER_H
#define CATCH23_REPORTER_H

#include "test_info.h"

#include "catchkit/expression_info.h"
#include "catchkit/result_type.h"
#include "catchkit/assertion_context.h"
#include "catchkit/report_on.h"

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

        virtual auto report_on_what() const -> ReportOn = 0;

        virtual void on_test_start( TestInfo const& test_info ) = 0;
        virtual void on_test_end( TestInfo const& test_info ) = 0;

        virtual void on_assertion_start( AssertionContext const& context ) = 0;
        virtual void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) = 0;
    };

} // namespace CatchKit

#endif // CATCH23_REPORTER_H
