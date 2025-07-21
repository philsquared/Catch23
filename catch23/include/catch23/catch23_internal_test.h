//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCH23_INTERNAL_TEST_H
#define CATCH23_INTERNAL_TEST_H

#include <functional>
#include <string>
#include <source_location>
#include <vector>

namespace CatchKit::Detail {
    struct Checker;

    struct Test {
        std::function<void(Checker&, Checker&)> test_fun;
        std::source_location location;
        std::string name = {};
        std::string tags = {};
    };
    std::vector<Test> const& get_all_tests();

    struct AutoReg {
        explicit AutoReg(Test&& test);
    };

} // CatchKit::Detail

#endif // CATCH23_INTERNAL_TEST_H
