//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCH23_INTERNAL_TEST_H
#define CATCH23_INTERNAL_TEST_H

#include "catch23_test_info.h"

#include <functional>
#include <vector>

namespace CatchKit::Detail {
    struct Checker;

    struct Test {
        std::function<void(Checker&, Checker&)> test_fun;
        TestInfo test_info;
    };
    std::vector<Test> const& get_all_tests();

    struct AutoReg {
        explicit AutoReg(Test&& test);
    };

} // CatchKit::Detail

#endif // CATCH23_INTERNAL_TEST_H
