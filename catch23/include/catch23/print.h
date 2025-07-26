//
// Created by Phil Nash on 26/07/2025.
//

#ifndef CATCH23_PRINT_H
#define CATCH23_PRINT_H

#include "colour.h"

#include <print>

namespace CatchKit {

    template<typename ...Args>
    void print( Colours colour, std::format_string<Args...> fmt, Args&&... args ) {
        TextColour colourGuard(colour);
        std::print(fmt, std::forward<Args>(args)...);
    }
    template<typename ...Args>
    void println( Colours colour, std::format_string<Args...> fmt, Args&&... args ) {
        TextColour colourGuard(colour);
        std::println(fmt, std::forward<Args>(args)...);
    }

} // namespace CatchKit

#endif // CATCH23_PRINT_H
