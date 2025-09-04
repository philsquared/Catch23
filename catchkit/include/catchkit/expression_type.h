//
// Created by Phil Nash on 04/09/2025.
//

#ifndef CATCHKIT_EXPRESSION_TYPE_H
#define CATCHKIT_EXPRESSION_TYPE_H

namespace CatchKit {

    enum class ExpressionType {
        Unary,
        Binary,
        Match,
        Exception,
        Expectation
    };

} // namespace CatchKit

#endif // CATCHKIT_EXPRESSION_TYPE_H