//
// Created by Phil Nash on 21/07/2025.
//

#include "../include/catchkit/catchkit_stringify.h"

#include <sstream>
#include <iomanip>

namespace CatchKit::Detail {
    namespace {
        enum class Endian{ Big, Little };

        constexpr auto which_endian() {
            int one = 1;
            // If the lowest byte we read is non-zero, we can assume
            // that little endian format is used.
            auto value = *reinterpret_cast<char*>(&one); // !TBD use bit_cast?
            return value ? Endian::Little : Endian::Big;
        }
    }

    auto raw_memory_to_string(void const* object, std::size_t size ) -> std::string {
        // Reverse order for little endian architectures
        int i = 0, end = static_cast<int>( size ), inc = 1;
        if( which_endian() == Endian::Little ) {
            i = end-1;
            end = inc = -1;
        }

        unsigned char const *bytes = static_cast<unsigned char const *>(object);

        // !TBD: replace with something std::format based?
        std::ostringstream rss;
        rss << "0x" << std::setfill('0') << std::hex;
        for( ; i != end; i += inc )
            rss << std::setw(2) << static_cast<unsigned>(bytes[i]);
        return rss.str();
    }

    auto pointer_to_string(void const* p) -> std::string {
        return std::format("0x{:0>8}", reinterpret_cast<intptr_t>(p));
    }

} // namespace CatchKit::Detail
