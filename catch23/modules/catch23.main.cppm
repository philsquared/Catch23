// Module interface for Catch23 main entry point infrastructure
// This provides C++20 module support for test runner, reporters, and CLI
//
// This module is separate from catch23 so that multiple test executables
// can import the pre-compiled runner/reporter infrastructure without
// rebuilding it each time.
//
// Usage:
//   #include "catch23/catch23_main_mod.h"
//   // This imports catch23.main, catch23, and defines main() for you

module;

// Include runner, reporter, and related infrastructure
#include "catch23/runner.h"
#include "catch23/reporter.h"
#include "catch23/console_reporter.h"
#include "catch23/internal_test.h"
#include "catch23/print.h"
#include "catch23/colour.h"

export module catch23.main;

// Re-export catch23 module for convenience
export import catch23;

// Export main-related types and functions
export namespace CatchKit {
    using Detail::TestRunner;
    using CatchKit::Reporter;
    using CatchKit::ConsoleReporter;
}

// Export Detail types and functions needed by main macro
export namespace CatchKit::Detail {
    using Detail::Test;
    using Detail::get_all_tests;
    using Detail::range_of;
}
