#ifdef USE_CATCH23_MODULES
    #include "catch23/catch23_main_mod.h"
#else
    #include "catch23/main.h"
    #include "catch23/console_reporter.h"
    #include "catch23/runner.h"
#endif


CATCH23_MIN_MAIN();
// CATCH23_MIN_MAIN(CatchKit::ReportOn::AllResults);
// CATCH23_MIN_MAIN(CatchKit::ReportOn::FailingTests, CatchKit::PrintSummary::OnlyWhenTestsAreReported);
