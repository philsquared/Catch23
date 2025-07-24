//
// Created by Phil Nash on 24/07/2025.
//

#ifndef CATCH23_RANDOM_H
#define CATCH23_RANDOM_H
#include <random>
#include <type_traits>

namespace CatchKit::Detail {

    // !TBD This is a placeholder for a proper implementation
    // In particular the non-portable distribution is an issue
    // Should probably borrow from Martin's work on Catch2

    template<typename T>
    concept IsNumeric = std::integral<T> || std::floating_point<T>;

    // Returns a number between from and to, inclusive
    template<IsNumeric NumberT>
    auto generate_random_number(NumberT from, NumberT to) -> NumberT {
        std::random_device random_device;
        std::mt19937 mt(random_device());
        if constexpr(std::integral<NumberT>)
            return std::uniform_int_distribution<NumberT>(from, to)(mt);
        else
            return std::uniform_real_distribution<NumberT>(from, to)(mt);
    }

} // namespace CatchKit::Detail

#endif //CATCH23_RANDOM_H
