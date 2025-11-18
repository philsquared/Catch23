//
// Created by Phil Nash on 18/11/2025.
//

#ifndef CATCH23_TEST_REGISTRY_H
#define CATCH23_TEST_REGISTRY_H

#include "internal_test.h"

#include <vector>
#include <unordered_map>

namespace CatchKit::Detail {

    class TestRegistry {
        std::vector<Test> all_tests;
        std::unordered_map<std::string, std::size_t> tests_by_name;
    public:
        explicit TestRegistry( std::vector<Test>&& tests );

        auto get_all_tests() const -> std::vector<Test> const& {
            return all_tests;
        }
        auto find_test_by_name( std::string const& name ) const -> Test const*;
    };

    void register_test( Test&& test );
    auto get_test_registry() -> TestRegistry const&;

} // namespace CatchKit

namespace CatchKit {
    using Detail::TestRegistry;
    using Detail::get_test_registry;
}

#endif // CATCH23_TEST_REGISTRY_H