//
// Created by Phil Nash on 19/09/2025.
//

#ifndef CATCH23_ADJUSTED_RESULT_H
#define CATCH23_ADJUSTED_RESULT_H

namespace CatchKit {

    // After taking account of may_fail and should_fail tags
    enum class AdjustedResult {
        Unknown,
        Passed,
        FailedExpectly, // Failed, but may_fail was in operation
        Failed
    };
} // namespace CatchKit

#endif // CATCH23_ADJUSTED_RESULT_H