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
        enum class Type {
            normal,
            manual, // test doesn't run by default, but can be run explicitly
            mayfail, // If test fails, don't count it as a failed run overall
            shouldfail, // If test fails count it as a pass. If it passes count as a failure.
        };
        std::string name;
        Type type = Type::normal;
        bool ignored = false; // This means "pretend this tag doesn't exist" and is set by !

        auto operator!() const -> Tag {
            return Tag{name, type, !ignored};
        }
    };

    struct TestInfo {
        std::source_location location;
        std::string name = {};
        std::vector<Tag> tags = {};

        auto is_manual() const -> bool;
        auto should_fail() const -> bool;
        auto may_fail() const -> bool;

    };

} // namespace CatchKit

#endif // CATCH23_TEST_INFO_H
