//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_TEST_INFO_H
#define CATCH23_TEST_INFO_H

#include <source_location>
#include <string>

namespace CatchKit {

    struct TestInfo {
        std::source_location location;
        std::string name = {};
        std::string tags = {};
    };

} // namespace CatchKit

#endif //CATCH23_TEST_INFO_H
