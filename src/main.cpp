#include "catch23/catch23_console_reporter.h"
#include "catch23/catch23_runner.h"

int main() {
    CatchKit::ConsoleReporter reporter(CatchKit::ReportOn::FailuresOnly);
    run_tests(CatchKit::Detail::get_all_tests(), reporter);
}