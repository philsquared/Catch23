//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/internal_test.h"

#include <vector>
#include <unordered_map>

namespace CatchKit::Detail {
    namespace {
        auto& get_all_tests_impl() {
            static std::vector<Test> all_tests;
            return all_tests;
        }
        auto& get_tests_by_name_impl() {
            static std::unordered_map<std::string, std::size_t> tests_by_name;
            return tests_by_name;
        }
    }


    std::vector<Test> const& get_all_tests() { return get_all_tests_impl(); }

    void register_test(Test&& test) {
        auto& all_tests = get_all_tests_impl();
        all_tests.emplace_back(std::move(test));
        get_tests_by_name_impl().emplace( all_tests.back().test_info.name, all_tests.size()-1 );
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
