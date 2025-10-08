//
// Created by Phil Nash on 23/09/2025.
//

#ifndef CATCHKIT_REFLECTION_H
#define CATCHKIT_REFLECTION_H

#include <string>
#include <source_location>
#include <string_view>

namespace CatchKit {
    namespace Detail {
        auto parse_templated_name( std::string const& templated_name, std::source_location location = std::source_location::current() ) -> std::string_view;
        template<typename T>
        auto type_to_string() -> std::string_view {
            // Handle common built-in types directly to save instantiating a std::source_location
            if constexpr( std::is_same_v<T, int> )
                return "int";
            if constexpr( std::is_same_v<T, bool> )
                return "bool";
            if constexpr( std::is_same_v<T, float> )
                return "float";
            if constexpr( std::is_same_v<T, double> )
                return "double";
            if constexpr( std::is_same_v<T, std::string> )
                return "std::string";
            if constexpr( std::is_same_v<T, std::string_view> )
                return "std::string_view";
            else
                return parse_templated_name("T");
        }

        auto parse_enum_name_from_function(std::string_view function_name, bool fully_qualified = false) -> std::string_view;
        auto unknown_enum_to_string(size_t enum_value) -> std::string;

        constexpr std::size_t enum_probe_start = 0;
        constexpr std::size_t enum_probe_end = 16;

        template<typename E, E candidate=static_cast<E>(enum_probe_start), size_t max_probe=enum_probe_end>
        struct enum_value_string {
            static constexpr auto find(E e) -> std::string_view {
                if( e == candidate )
                    return parse_enum_name_from_function(std::source_location::current().function_name());
                if constexpr(constexpr auto raw_value = static_cast<std::size_t>( candidate );
                    raw_value < max_probe &&
                    requires { std::integral_constant<E, static_cast<E>(raw_value+1)>{}; } ) {
                        return enum_value_string<E, static_cast<E>(raw_value + 1)>::find(e);
                }
                return {};
            }
        };

        template<typename E>
        auto constexpr enum_to_string(E e) -> std::string {
            if( auto val = enum_value_string<E>::find(e); !val.empty() )
                return std::string(val);
            return unknown_enum_to_string(static_cast<size_t>(e));
        }
        auto normalise_type_name(std::string_view type_name) -> std::string;

    } // namespace Detail

    using Detail::type_to_string;
    using Detail::enum_to_string;

} // namespace CatchKit

#endif // CATCHKIT_REFLECTION_H