//
// Created by Phil Nash on 21/07/2025.
//

#include "catch23/catch23_internal_test.h"

namespace CatchKit::Detail {
    std::vector<Test> tests;
    std::vector<Test> const& get_all_tests() { return tests; }

    AutoReg::AutoReg(Test&& test) {
        tests.emplace_back(std::move(test));
    }
}
