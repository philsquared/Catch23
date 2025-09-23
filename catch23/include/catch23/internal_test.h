//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCH23_INTERNAL_TEST_H
#define CATCH23_INTERNAL_TEST_H

#include "test_info.h"

#include <functional>
#include <vector>

namespace CatchKit::Detail {
    struct Checker;

    template<typename T>
    concept TagConvertible = std::constructible_from<Tag, T>;

    auto make_test_info(std::source_location location, std::string name = {}, std::string_view tag_spec = {} ) -> TestInfo;

    struct Test {
        std::function<void(Checker&, Checker&)> test_fun;
        TestInfo test_info;

        template<TagConvertible... T>
        auto&& operator[](T&&...tags) {
            test_info.tags = std::vector{Tag{std::forward<T>(tags)}...};
            return std::move(*this);
        }
    };
    std::vector<Test> const& get_all_tests();
    auto find_test_by_name(std::string const& name) -> Test const*;

    struct AutoReg {
        explicit AutoReg(Test&& test);
    };

} // CatchKit::Detail

namespace CatchKit::Tags {
    // Muted tests are not run by default, but can be run by name or tag
    inline constexpr Tag mute{"^mute", Tag::Type::mute };

    // If any tests are "soloed" then a default run will _only_ run the soloed tests
    // (solo overrides mute, so if both tags are present, the test is run)
    inline constexpr Tag solo{"^solo", Tag::Type::solo };

    // Tests that may_fail will be counted as a pass for the overall run even if they fail
    inline constexpr Tag may_fail{"^may_fail", Tag::Type::may_fail };

    // Tests that should_fail are counted as a pass for the overall run if they fail, and a fail if they pass
    inline constexpr Tag should_fail{"^should_fail", Tag::Type::should_fail };

    // Tests that always_report will report successful tests regardless of flags
    inline constexpr Tag always_report{"^always_report", Tag::Type::always_report };
}

#endif // CATCH23_INTERNAL_TEST_H
