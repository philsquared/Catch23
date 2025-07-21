//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_RESULT_TYPE_H
#define CATCHKIT_RESULT_TYPE_H

namespace CatchKit {

    enum class ResultType {
        Unknown,
        Pass,
        ExpressionFailed,
        MatchFailed,
        UnexpectedException,
        MissingException
    };

} // namespace CatchKit

#endif //CATCHKIT_RESULT_TYPE_H
