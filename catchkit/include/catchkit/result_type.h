//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_RESULT_TYPE_H
#define CATCHKIT_RESULT_TYPE_H

namespace CatchKit {

    enum class ResultType {
        Passed,
        Failed
    };

    inline bool operator! (ResultType result) {
        return result == ResultType::Failed;
    }

} // namespace CatchKit

#endif // CATCHKIT_RESULT_TYPE_H
