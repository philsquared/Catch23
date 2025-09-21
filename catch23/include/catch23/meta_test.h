//
// Created by Phil Nash on 24/07/2025.
//

#ifndef CATCH23_META_TEST_H
#define CATCH23_META_TEST_H

#include "internal_test.h"
#include "test_result_handler.h"

#include "catchkit/checker.h"

namespace CatchKit {

    struct FullAssertionInfo {
        AssertionContext context;
        AssertionInfo info;

        [[nodiscard]] auto passed() const { return info.passed(); }
        [[nodiscard]] auto failed() const { return info.failed(); }
        [[nodiscard]] auto failed_expectedly() const { return info.result == AdjustedResult::FailedExpectly; }
    };

    class MetaTestReporter : public Reporter {
        ReportOn what_to_report_on;

    public:
        explicit MetaTestReporter( ReportOn what_to_report_on = ReportOn::AllResults )
        : what_to_report_on( what_to_report_on )
        {}

        [[nodiscard]] auto report_on_what() const -> ReportOn override {
            return what_to_report_on;
        }
        void on_test_run_start() override {}
        void on_test_run_end() override {}

        void on_test_start( TestInfo const& ) override {}
        void on_test_end( TestInfo const&, Counters const& ) override {}

        void on_assertion_start( AssertionContext const& ) override {}
        void on_assertion_end( AssertionContext const& context, AssertionInfo const& assertion_info ) override {
            results.emplace_back(context, assertion_info);
        }
        void on_shrink_start() override {}
        void on_shrink_found( std::vector<std::string> const&, int ) override {}
        void on_no_shrink_found( int ) override {}
        void on_shrink_result( ResultType, int ) override {}
        void on_shrink_end() override {}

        std::vector<FullAssertionInfo> results;
    };

    struct MetaTestResults {
        std::vector<FullAssertionInfo> all_results;

        [[nodiscard]] auto size() const { return all_results.size(); }
        [[nodiscard]] auto& operator[](std::size_t index) const { return all_results.at(index); }
        [[nodiscard]] auto failed() const {
            return !all_results.empty() && all_results.back().failed();
        }
        [[nodiscard]] auto passed() const {
            return !all_results.empty() && all_results.back().passed();
        }
        [[nodiscard]] auto message() const {
            return !all_results.empty() ? all_results.back().info.message : std::string();
        }
        [[nodiscard]] auto failures() const -> int;
        [[nodiscard]] auto expected_failures() const -> int;
    };

    class MetaTestRunner {
        MetaTestReporter reporter;

        std::string name;
        std::source_location location;

    public:
        explicit MetaTestRunner(std::string name = "local test", std::source_location location = std::source_location::current());
        auto run( Detail::Test const& test ) && -> MetaTestResults;
        auto run_test_by_name( std::string const& name ) && -> MetaTestResults;

        auto operator << ( std::invocable<Checker&, Checker&> auto const& test_fun ) && {
            return std::move(*this).run(Detail::Test{test_fun, {location, std::move(name)}});
        }
    };

} // namespace CatchKit

#define LOCAL_TEST(...) CatchKit::MetaTestRunner(__VA_ARGS__) << [](CatchKit::Checker& check, CatchKit::Checker&)
#define RUN_TEST_BY_NAME(name) CatchKit::MetaTestRunner(name).run_test_by_name( name )

#endif // CATCH23_META_TEST_H
