//
// Created by Phil Nash on 21/07/2025.
//

#include "../include/catchkit/matchers.h"

#include <algorithm>

namespace CatchKit {

    // !TBD: test the others
    static_assert(Detail::IsMatcher<GenericMatchers::Equals<bool>>);
    // static_assert(Detail::IsEagerMatcher<GenericMatchers::Equals<char[7]>>);

    namespace StringMatchers {

        bool CaseSensitive::equal(std::string_view str1, std::string_view str2) {
            return str1 == str2;
        }
        bool CaseSensitive::find(std::string_view str, std::string_view substr) {
            return str.find(substr) != std::string_view::npos;
        }
        bool CaseInsensitive::equal(std::string_view str1, std::string_view str2) {
            return std::ranges::equal(str1, str2,
                [](char a, char b) {
                    return std::tolower(a) == std::tolower(b);
                });
        }
        bool CaseInsensitive::find(std::string_view str, std::string_view substr) {
            return !std::ranges::search(str, substr,
                [](char a, char b) {
                    return std::tolower(a) == std::tolower(b);
                }).empty();
        }

    } // namespace StringMatchers

} // namespace CatchKit