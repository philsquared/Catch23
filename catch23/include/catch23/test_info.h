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
            mute, // test doesn't run by default, but can be run explicitly
            solo, // Only soloed tests are run by default
            may_fail, // If test fails, don't count it as a failed run overall
            should_fail, // If test fails count it as a pass. If it passes count as a failure.
            always_report, // Report all tests, even successful ones, regardless of flags
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

        auto has_tag_type(Tag::Type tag_type) const -> bool;
        auto should_fail() const -> bool;
        auto may_fail() const -> bool;
    };

} // namespace CatchKit

#endif // CATCH23_TEST_INFO_H
