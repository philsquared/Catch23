#include "catch23/catch23_console_reporter.h"
#include "catch23/catch23_test.h"
#include "catch23/catch23_runner.h"

#include "catchkit/catchkit_matchers.h"


TEST("test a") {
    CHECK( 1 == 2 );
    throw std::domain_error("I didn't expect that!");
}

TEST("test b") {
    REQUIRE_THAT("one", equals("one"));
}


int main() {
    CatchKit::ConsoleReporter reporter;
    run_tests(CatchKit::Detail::get_all_tests(), reporter);
}