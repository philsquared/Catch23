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

    struct Counters {
        int passed_explicitly = 0;
        int failed_expectedly = 0;
        int failed = 0;

        [[nodiscard]] auto passed() const { return passed_explicitly + failed_expectedly; }

        [[nodiscard]] auto total() const { return passed() + failed; }

        [[nodiscard]] auto all_passed() const { return failed == 0; }

        auto& operator += ( Counters const& other ) {
            passed_explicitly += other.passed_explicitly;
            failed_expectedly += other.failed_expectedly;
            failed += other.failed;

            return *this;
        }

        friend auto operator + ( Counters const& lhs, Counters const& rhs ) {
            Counters result = lhs;
            result += rhs;
            return result;
        }
    };

    struct Reporter {
        virtual ~Reporter() = default;

        virtual auto report_on_what() const -> ReportOn = 0;

        virtual void on_test_start( TestInfo const& test_info ) = 0;
        virtual void on_test_end( TestInfo const& test_info, Counters const& assertions ) = 0;

        virtual void on_assertion_start( AssertionContext const& context ) = 0;
        virtual void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) = 0;

        virtual void on_shrink_start() = 0;
        virtual void on_shrink_found( std::vector<std::string> const& values, int shrinks ) = 0;
        virtual void on_no_shrink_found( int shrinks ) = 0;
        virtual void on_shrink_result( ResultType result, int shrinks_so_far ) = 0;

    };

} // namespace CatchKit

#endif // CATCH23_REPORTER_H
