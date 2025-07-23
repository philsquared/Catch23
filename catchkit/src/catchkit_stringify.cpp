//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/catchkit_stringify.h"

#include <sstream>
#include <cassert>

namespace CatchKit::Detail {

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

    auto pointer_to_string(void const* p) -> std::string {
        return std::format("0x{:0>8}", reinterpret_cast<intptr_t>(p));
    }

} // namespace CatchKit::Detail
