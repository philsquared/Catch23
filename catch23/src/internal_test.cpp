//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/internal_test.h"

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <ranges>

namespace CatchKit::Detail {
    namespace {
        auto& get_all_tests_impl() {
            static std::vector<Test> all_tests; // NOSONAR NOLINT (misc-typo)
            return all_tests;
        }
        auto& get_tests_by_name_impl() {
            static std::unordered_map<std::string, std::size_t> tests_by_name; // NOSONAR NOLINT (misc-typo)
            if( tests_by_name.empty() ) {
                for(auto&& [i, test] : std::views::enumerate( get_all_tests_impl() ) ) {
                    tests_by_name.try_emplace( test.test_info.name, i );
                }
            }
            return tests_by_name;
        }
    }

    // This is a legacy, compatibility, feature that should probably be moved out of the core
    auto make_test_info(std::source_location location, std::string name, std::string_view tag_spec ) -> TestInfo {
        std::vector<Tag> tags;
        while(true) {
            auto start = tag_spec.find('[');
            if( start == std::string::npos )
                break;
            auto end = tag_spec.find(']', start);
            if( end == std::string::npos )
                throw std::invalid_argument("Invalid tag specification - missing an expected `]`");
            std::string tag_name(tag_spec.substr( start+1, end-start-1 ));
            if( tag_name == "!mayfail" ) // NOLINT (misc-typo)
                tags.emplace_back(Tags::may_fail);
            if( tag_name == "!shouldfail" ) // NOLINT (misc-typo)
                tags.emplace_back(Tags::should_fail);
            else if( tag_name.starts_with( "." ) )
                tags.emplace_back(tag_name, Tag::Type::mute);
            else
                tags.emplace_back(std::move(tag_name));
            tag_spec = tag_spec.substr(end);
        }
        return { location, std::move(name), std::move(tags) };
    }

    std::vector<Test> const& get_all_tests() { return get_all_tests_impl(); }

    void register_test(Test&& test) {
        auto& all_tests = get_all_tests_impl();
        all_tests.emplace_back(std::move(test));
    }
    auto find_test_by_name(std::string const& name) -> Test const* {
        if( auto it = get_tests_by_name_impl().find( name ); it != get_tests_by_name_impl().end() )
            return &get_all_tests_impl()[it->second];
        return nullptr;
    }

    AutoReg::AutoReg(Test&& test) {
        register_test(std::move(test));
    }
}
