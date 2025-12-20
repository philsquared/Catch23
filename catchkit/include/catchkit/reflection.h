//
// Created by Phil Nash on 23/09/2025.
//

#ifndef CATCHKIT_REFLECTION_H
#define CATCHKIT_REFLECTION_H

#include <cassert>
#include <string>
#include <source_location>
#include <string_view>
#include <format>
#include <utility>

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
        auto normalise_type_name(std::string_view type_name) -> std::string;

        // Converts this compile-time known enum case to a string
        template<auto EC>
        consteval auto enum_case_to_string() -> std::string_view {
            std::string_view fname = std::source_location::current().function_name();
            if( auto start = fname.find("EC = "); start != std::string_view::npos ) {
                start += 5;
                auto end = fname.find_first_of(";]", start);
                assert( end != std::string_view::npos ); // If a compiler error leads here we need to fix the parsing
                return fname.substr(start, end-start);
            }
            return {};
        }
        consteval auto is_valid_enum_case(std::string_view name) -> bool {
            return !name.empty() && name[0] != '(';
        }

        auto unknown_enum_to_string(std::integral auto enum_value) -> std::string {
            return std::format("<unknown enum value: {}>", enum_value);
        }
        auto remove_qualification(std::string_view qualified_name) -> std::string_view;

        constexpr int enum_probe_start = 0;
        constexpr int enum_sparse_probe_end = 16;
        constexpr int enum_sequential_probe_end = 64;

        template<typename E,
            int direction,
            int probe,
            int sparse_probe_end,
            int sequential_probe_end>
        struct enum_value_string {
            using UnderlyingType = std::underlying_type_t<E>;
            static constexpr UnderlyingType candidate = static_cast<UnderlyingType>(probe*direction);
            static constexpr auto find(E value) -> std::string_view { return find(std::to_underlying(value)); }
            static constexpr auto find(UnderlyingType underlying) -> std::string_view {
                if constexpr( requires { std::integral_constant<E, static_cast<E>( candidate )>{}; } ) {
                    constexpr auto case_name = enum_case_to_string<static_cast<E>( candidate )>();
                    if( underlying == candidate )
                        return remove_qualification( case_name );
                    if constexpr( ( probe < sparse_probe_end || is_valid_enum_case( case_name ) )
                            && probe <sequential_probe_end ) {
                        return enum_value_string<E, direction, probe+1, sparse_probe_end, sequential_probe_end>::find(underlying);
                    }
                }
                return {};
            }
        };

        // Convert a runtime enum case value to a string
        template<
                int sparse_probe_end=enum_sparse_probe_end,
                int sequential_probe_end=enum_sequential_probe_end,
                int probe_start=enum_probe_start,
                typename E>
            requires std::is_enum_v<E>
        auto constexpr enum_to_string(E e) -> std::string {
            auto underlying = std::to_underlying(e);
            if constexpr( std::is_signed_v<std::underlying_type_t<E>> ) {
                if( underlying < 0 )
                    if( auto name = enum_value_string<E, -1, probe_start+1, sparse_probe_end, sequential_probe_end>::find(e); !name.empty() )
                        return std::string(name);
            }
            if( auto name = enum_value_string<E, 1, probe_start, sparse_probe_end, sequential_probe_end>::find(e); !name.empty() )
                return std::string(name);
            return unknown_enum_to_string( underlying );
        }

    } // namespace Detail

    using Detail::type_to_string;
    using Detail::enum_to_string;

} // namespace CatchKit

#endif // CATCHKIT_REFLECTION_H