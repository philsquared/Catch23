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

    Test::Test( std::function<void(Checker&, Checker&)>&& test_fun, TestInfo&& test_info )
    : test_fun(std::move(test_fun)), test_info(std::move(test_info))
    {}

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
