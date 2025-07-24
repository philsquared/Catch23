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

    // Returns a number between from and to, inclusive
    template<std::integral IntegerT>
    auto generate_random_number(IntegerT from, IntegerT to) -> IntegerT {
        std::random_device random_device;
        std::mt19937 mt(random_device());
        std::uniform_int_distribution<IntegerT> uniform_dist(from, to);
        return uniform_dist(mt);
    }

} // namespace CatchKit::Detail

#endif //CATCH23_RANDOM_H
