#ifdef USE_CATCH23_MODULES
    #include "catch23/catch23_main_mod.h"
#else
    #include "catch23/main.h"
    #include "catch23/console_reporter.h"
    #include "catch23/runner.h"
    #include "catch23/config.h"
    #include "catch23/command_line.h"
#endif

// CATCH23_MIN_MAIN();

int main(int argc, char** argv) {
    using namespace CatchKit;
    auto maybe_config = parse_config(argc, argv);
    if( !maybe_config )
        return maybe_config.error();

    auto const& config = *maybe_config;

    // !TBD: Choose reporter based on config:
    auto reporter = CatchKit::ConsoleReporter( config.show_successful_tests ? ReportOn::AllResults : ReportOn::FailingTests );

    CatchKit::TestRunner runner(reporter, config);
    runner.run_tests(CatchKit::get_test_registry());
}


// CATCH23_MIN_MAIN(CatchKit::ReportOn::AllResults);
// CATCH23_MIN_MAIN(CatchKit::ReportOn::FailingTests, CatchKit::PrintSummary::OnlyWhenTestsAreReported);
