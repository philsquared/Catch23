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
    auto remove_qualification(std::string_view qualified_name) -> std::string_view {
        if( auto last_colon = qualified_name.find_last_of(':'); last_colon != std::string_view::npos )
            return qualified_name.substr(last_colon+1);
        return qualified_name; // NOLINT
    }

    auto normalise_type_name(std::string_view type_name) -> std::string {
        if( type_name.starts_with("std::") ) {
            auto substr = type_name.substr(5);
            if( auto pos = substr.find( "basic_" ); pos != std::string_view::npos ) {
                substr = substr.substr(pos+6);
                if( substr.starts_with( "string<wchar_t>" ) )
                    return "std::wstring";
                if( substr.starts_with( "string_view<wchar_t>" ) )
                    return "std::wstring_view";

                if( substr.starts_with( "string<char8_t>" ) )
                    return "std::u8string";
                if( substr.starts_with( "string_view<char8_t>" ) )
                    return "std::u8string_view";

                if( substr.starts_with( "string<char16_t>" ) )
                    return "std::u16string";
                if( substr.starts_with( "string_view<char16_t>" ) )
                    return "std::u16string_view";
            }
        }
        return std::string(type_name);
    }

} // namespace CatchKit::Detail
