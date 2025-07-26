#include "catch23/console_reporter.h"
#include "catch23/runner.h"

int main() {
    CatchKit::ConsoleReporter reporter(CatchKit::ReportOn::FailuresOnly);

    std::string only_run_test; // = "section within a generator";

    if( only_run_test.empty()) {
        run_tests(CatchKit::Detail::get_all_tests(), reporter);
    }
    else {
        std::vector<CatchKit::Detail::Test> const& tests = CatchKit::Detail::get_all_tests();
        std::vector<CatchKit::Detail::Test> tests_to_run;
        for (auto const& test : tests) {
            if( test.test_info.name == only_run_test )
                tests_to_run.push_back(test);
        }
        assert(!tests_to_run.empty());
        run_tests(tests_to_run, reporter);
    }

    reporter.on_test_run_end();
}
