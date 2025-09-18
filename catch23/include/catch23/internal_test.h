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

    inline auto make_test_info(std::source_location location, std::string name = {}, std::string tag_spec = {} ) {
        return TestInfo{ location, std::move(name), {Tag(std::move(tag_spec))} };
    }

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
    inline constexpr Tag manual("[.]");
}

#endif // CATCH23_INTERNAL_TEST_H
