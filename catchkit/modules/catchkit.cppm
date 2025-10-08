// Module interface for CatchKit
// This provides C++20 module support alongside traditional headers
//
// IMPORTANT: Macros cannot be exported from C++20 modules!
// When using this module, you must also include "catchkit/macros.h" to get
// the CHECK, REQUIRE, CHECK_THAT, REQUIRE_THAT, and other assertion macros.
//
// Usage:
//   #include "catchkit/macros.h"
//   import catchkit;

module;

#include "catchkit/result_handler.h"
#include "catchkit/assert_result_handler.h"
#include "catchkit/result_type.h"
#include "catchkit/operators.h"
#include "catchkit/expression_info.h"
#include "catchkit/assertion_context.h"
#include "catchkit/exceptions.h"
#include "catchkit/reflection.h"
#include "catchkit/variable_capture.h"
#include "catchkit/expr_ref.h"
#include "catchkit/report_on.h"
#include "catchkit/stringify.h"
#include "catchkit/matchers.h"
#include "catchkit/internal_matchers.h"
#include "catchkit/checker.h"

export module catchkit;

export namespace CatchKit {
    using CatchKit::MatchResult;
    using CatchKit::stringify;

    using Detail::Checker;
    using Detail::Asserter;
    using Detail::ResultHandler;
    using Detail::AssertResultHandler;
    using Detail::AssertionContext;
}
export namespace CatchKit::Detail {
    using Detail::Asserter;
    using Detail::TypedVariableCaptureRef;
}
export namespace CatchKit::GenericMatchers {}
export namespace CatchKit::StringMatchers {}
export namespace CatchKit::FloatMatchers {}
export namespace CatchKit::VectorMatchers {}
export namespace CatchKit::ExceptionMatchers {
    using ExceptionMatchers::HasMessage;
}

export namespace CatchKit::Matchers {
    using Matchers::contains;
    using Matchers::starts_with;
    using Matchers::ends_with;
    using Matchers::equals;
    using Matchers::has_size;
    using Matchers::is_close_to;
    using Matchers::is_true;
    using Matchers::is_false;
    using Matchers::matches_predicate;
    using Matchers::throws;

    using StringMatchers::CaseSensitive;
    using StringMatchers::CaseInsensitive;

    using Detail::operator&&;
    using Detail::operator||;
    using Detail::operator!;
    using Detail::operator>>=;
}
