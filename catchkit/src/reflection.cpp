//
// Created by Phil Nash on 23/09/2025.
//

#include "catchkit/reflection.h"

#include <format>

namespace CatchKit::Detail {

    auto parse_templated_name_from_function_name( std::string const& templated_name, std::string_view function_name ) -> std::string_view {
        if( auto start = function_name.find(templated_name + " = "); start != std::string_view::npos ) {
            start += templated_name.size() + 3;
            if( auto end = function_name.find_first_of("];", start); end != std::string_view::npos )
                return function_name.substr(start, end-start);
        }
        return {};
    }
    auto parse_templated_name( std::string const& templated_name, std::source_location location ) -> std::string_view {
        return parse_templated_name_from_function_name( templated_name, location.function_name() );
    }
    auto parse_enum_name_from_function(std::string_view function_name, bool fully_qualified) -> std::string_view {
        auto qualified_enum_name = parse_templated_name_from_function_name("candidate", function_name);
        if( auto comma = qualified_enum_name.find_first_of(','); comma != std::string_view::npos )
            qualified_enum_name = qualified_enum_name.substr(0, comma);
        if( fully_qualified )
            return qualified_enum_name;
        if( auto last_colon = qualified_enum_name.find_last_of(':'); last_colon != std::string_view::npos )
            return qualified_enum_name.substr(last_colon+1);
        return qualified_enum_name;
    }
    auto unknown_enum_to_string(size_t enum_value) -> std::string {
        return std::format("<unknown enum value: {}>", enum_value);
    }

} // namespace CatchKit::Detail
