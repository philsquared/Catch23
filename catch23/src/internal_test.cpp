//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/internal_test.h"
#include "catch23/test_registry.h"

#include <ranges>

namespace CatchKit::Detail {

    Test::Test( TestFunction&& test_fun, TestInfo&& test_info )
    : test_fun(std::move(test_fun)), test_info(std::move(test_info))
    {}

    AutoReg::AutoReg(Test&& test) {
        register_test(std::move(test));
    }
}
