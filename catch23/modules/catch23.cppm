// Module interface for Catch23
// This provides C++20 module support alongside traditional headers
//
// IMPORTANT: Macros cannot be exported from C++20 modules!
// When using this module, you must also include "catch23/macros.h" to get
// the TEST, CHECK, REQUIRE, SECTION, and GENERATE, etc macros.
//
// Usage:
//   #include "catch23/macros.h"
//   import catch23;

module;

#include "catch23/internal_test.h"
#include "catch23/runner.h"
#include "catch23/reporter.h"
#include "catch23/console_reporter.h"
#include "catch23/generators.h"
#include "catch23/sections.h"
#include "catch23/test_info.h"
#include "catch23/test_result_handler.h"
#include "catch23/colour.h"
#include "catch23/print.h"
#include "catch23/random.h"
#include "catch23/main.h"
#include "catch23/catch2_compat.h"
#include "catch23/meta_test.h"
#include "catch23/adjusted_result.h"
#include "catch23/generator_node.h"

export module catch23;

// Re-export CatchKit for convenience
export import catchkit;

export namespace CatchKit {
    using CatchKit::MetaTestRunner;

    using Detail::TestRunner;
    using Detail::TestResultHandler;
}
export namespace CatchKit::Detail {
    using Detail::try_enter_section;
    using Detail::RandomNumberGenerator;
    using Detail::operator,;
}

export namespace CatchKit::Tags {
    using Tags::mute;
    using Tags::solo;
    using Tags::may_fail;
    using Tags::should_fail;
    using Tags::always_report;
}

export namespace CatchKit::Generators {
    using Generators::generate;
    using Generators::values_of;
    using Generators::from_values;
    using Generators::inclusive_range_of;

    namespace Charsets = Detail::Charsets;
}

export namespace CatchKit::Detail::Charsets {
    using Detail::Charsets::symbols;
}

export namespace CatchKit::Detail {
    using Detail::AutoReg;
    using Detail::Test;
    using Detail::SectionInfo;
    using Detail::ExecutionNodes;
    using Detail::GeneratorAcquirer;
    using Detail::get_execution_nodes_from_result_handler;
    using Detail::make_test_info;
}
