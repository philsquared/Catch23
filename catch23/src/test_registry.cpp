//
// Created by Phil Nash on 18/11/2025.
//

#include "catch23/test_registry.h"

#include <optional>
#include <cassert>
#include <ranges>

namespace CatchKit::Detail {

    namespace {
        auto& get_registered_tests_impl() {
            static std::vector<Test> all_tests; // NOSONAR NOLINT (misc-typo)
            return all_tests;
        }
        bool& is_initialised() {
            static bool initialised = false;
            return initialised;
        }
    }
    void register_test( Test&& test ) {
        assert(!is_initialised()); // We can only call this during startup
        get_registered_tests_impl().emplace_back(std::move(test));
    }

    auto get_test_registry() -> TestRegistry const& {
        static std::optional<TestRegistry> all_tests;
        if( !is_initialised() ) {
            all_tests = TestRegistry{ std::move(get_registered_tests_impl()) };
            is_initialised() = true;
        }
        return *all_tests;
    }

    TestRegistry::TestRegistry( std::vector<Test>&& tests )
    :   all_tests( std::move( tests ) )
    {
        if( tests_by_name.empty() ) {
            for(auto&& [i, test] : std::views::enumerate( all_tests ) ) {
                tests_by_name.try_emplace( test.test_info.name, i );
            }
        }

    }

    auto TestRegistry::find_test_by_name(std::string const& name) const -> Test const* {
        if( auto it = tests_by_name.find( name ); it != tests_by_name.end() )
            return &all_tests[it->second];
        return nullptr;
    }
    auto TestRegistry::find_tests_by_tag(std::string tag_name) const -> std::generator<Test const*> {
        for(auto const& test : all_tests) {
            for( auto const& tag : test.test_info.tags ) {
                if( tag.name == tag_name ) {
                    co_yield &test;
                }
            }
        }
    }

} // namespace CatchKit::Detail