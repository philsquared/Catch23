//
// Created by Phil Nash on 16/12/2025.
//

#ifndef CATCH23_CONFIG_H
#define CATCH23_CONFIG_H

#include <string>

namespace CatchKit {

    struct Config {
        bool show_successful_tests = false;
        bool break_into_debugger = false;
        std::string tests_or_tags;
        std::string reporter;
        bool help = false;
    };

} // namespace CatchKit

#endif // CATCH23_CONFIG_H