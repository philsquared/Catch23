//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_TEST_INFO_H
#define CATCH23_TEST_INFO_H

#include <source_location>
#include <string>
#include <vector>

namespace CatchKit {

    struct Tag {
        std::string name;
        constexpr Tag() = default;
        explicit constexpr Tag(std::string name) : name(std::move(name)) {}
    };

    struct TestInfo {
        std::source_location location;
        std::string name = {};
        std::vector<Tag> tags = {};
    };

} // namespace CatchKit

#endif // CATCH23_TEST_INFO_H
