//
// Created by Phil Nash on 08/10/2025.
//

#ifndef CATCHKIT_VARIABLE_CAPTURE_H
#define CATCHKIT_VARIABLE_CAPTURE_H

#include <string>

namespace CatchKit {

    struct CapturedVariable {
        std::string name;
        std::string type;
        std::string value;
    };

} // namespace CatchKit

#endif // CATCHKIT_VARIABLE_CAPTURE_H
