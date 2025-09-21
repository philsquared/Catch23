//
// Created by Phil Nash on 31/07/2025.
//

#ifndef CATCH23_MAIN_H
#define CATCH23_MAIN_H

// A minimal main that just uses the console reporter.
// Pass ReportOn::AllResults if you want to see passing tests broken down, too
#define CATCH23_MIN_MAIN(...) \
    int main() { \
        auto reporter = CatchKit::ConsoleReporter(__VA_ARGS__); \
        CatchKit::TestRunner runner(reporter); \
        runner.run_tests(CatchKit::Detail::get_all_tests()); \
    }

#ifdef CATCH23_IMPL_MIN_MAIN
#define CATCHKIT_IMPL
#endif

#endif // CATCH23_MAIN_H
