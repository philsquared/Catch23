#include "catch23/catch23_test.h"
#include "catch23/catch23_test_result_handler.h"

#include "catchkit/catchkit_matchers.h"

#include <print>

TEST("test a") {
    CHECK( 1 == 2 );
}

TEST("test b") {
    REQUIRE_THAT("one", equals("two"));
}

void run_tests()
{
    CatchKit::TestResultHandler test_handler;
    CatchKit::Checker check{ test_handler, CatchKit::ResultDisposition::Continue };
    CatchKit::Checker require{ test_handler, CatchKit::ResultDisposition::Abort };

    for( auto&& test : CatchKit::Detail::get_all_tests()) {
        std::println("Running `{}`", test.name);
        try {
            test.test_fun(check, require);
        }
        catch(...) {
            std::println("  *** aborted");
        }
    }

}
int main() {
    run_tests();
}