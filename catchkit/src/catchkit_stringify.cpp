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

    auto parse_enum_name_from_function(std::string_view function_name, bool fully_qualified) -> std::string_view {
        auto start = function_name.find("candidate = ");
        if (start != std::string_view::npos) {
            start += 12;
            auto end = function_name.find_first_of(",;", start);
            if (end != std::string_view::npos) {
                auto qualified_enum_name = function_name.substr(start, end-start);
                if( fully_qualified )
                    return qualified_enum_name;
                auto last_colon = qualified_enum_name.find_last_of(':');
                return qualified_enum_name.substr(last_colon+1);
            }
        }
        return {};
    }
    auto unknown_enum_to_string(size_t enum_value) -> std::string {
        return std::format("<unknown enum value: {}>", enum_value);
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
