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

    Config config;
    auto parser = make_cli_parser(config);

    if( auto result = parser.parse(argc, argv); !result ) {
        std::println("Invalid command line");
        return 1;
    }
    if( config.help ) {
        std::println("Help!" );
        // !TBD: show usage
        return 0;
    }

    // !TBD: Choose reporter based on config:
    auto reporter = ConsoleReporter( config.show_successful_tests ? ReportOn::AllResults : ReportOn::FailingTests );

    TestRunner runner(reporter, config);
    runner.run_tests(get_test_registry());
}


// CATCH23_MIN_MAIN(CatchKit::ReportOn::AllResults);
// CATCH23_MIN_MAIN(CatchKit::ReportOn::FailingTests, CatchKit::PrintSummary::OnlyWhenTestsAreReported);
