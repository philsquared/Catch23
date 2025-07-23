//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_STRINGIFY_H
#define CATCHKIT_STRINGIFY_H

#include <string>
#include <string_view>
#include <source_location>

#define CATCHKIT_FALLBACK_TO_FORMAT_STRING_CONVERSIONS
// #define CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS

#ifdef CATCHKIT_FALLBACK_TO_FORMAT_STRING_CONVERSIONS
#include <format>
#endif
#ifdef CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
#include <sstream>
#endif

namespace CatchKit {

    namespace Detail {
        auto raw_memory_to_string(void const* object, std::size_t size ) -> std::string;
        auto pointer_to_string(void const* p) -> std::string;
        void ignore( auto&& ) noexcept {}

        auto parse_enum_name_from_function(std::string_view function_name, bool fully_qualified = false) -> std::string_view;
        auto unknown_enum_to_string(size_t enum_value) -> std::string;

        constexpr size_t max_enum_scan = 16;

        template<typename E, E candidate=static_cast<E>(0)>
        struct enum_value_string {
            static auto find(E e) -> std::string_view {
                if(e == candidate) {
                    return parse_enum_name_from_function(std::source_location::current().function_name());
                }
                if constexpr(static_cast<size_t>(candidate) < max_enum_scan) {
                    return enum_value_string<E, static_cast<E>(static_cast<size_t>(candidate)+1)>::find(e);
                }
                else {
                    return {};
                }
            }
        };

        template<typename E>
        auto enum_to_string(E e) -> std::string {
            auto val = enum_value_string<E>::find(e);
            if( val.empty() )
                return unknown_enum_to_string(static_cast<size_t>(e));
            return std::string(val);
        }

    }

    #ifdef FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
    template <typename T>
    concept Streamable = requires( std::ostream os, T value ) {
        { os << value };
    };
    #endif

    template<typename T>
    struct Stringifier {
        [[nodiscard]] static auto stringify( T const& value ) -> std::string {

            #ifdef FALLBACK_TO_FORMAT_STRING_CONVERSIONS
            if constexpr( std::formattable<T, char> ) {
                return std::format("{}", value);
            }
            #endif

            #ifdef FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
            if constexpr ( Streamable<T> ) {
                std::ostringstream oss; // !TBD: use an ostringstream pool?
                oss << value;
                return oss.str();
            }
            #endif

            #if !defined(FALLBACK_TO_OSTREAM_STRING_CONVERSIONS) && !defined(FALLBACK_TO_FORMAT_STRING_CONVERSIONS)
            Detail::ignore( value );
            #endif

            return "{?}";
        }
    };

    template<>
    struct Stringifier<std::string> {
        [[nodiscard]] static constexpr auto stringify( std::string const& value ) -> std::string { return value; }
        [[nodiscard]] static constexpr auto stringify( std::string&& value ) -> std::string { return std::move(value); }
    };
    template<>
    struct Stringifier<std::string_view> {
        [[nodiscard]] static constexpr auto stringify( std::string_view value ) -> std::string { return std::string( value ); }
    };
    template<>
    struct Stringifier<char const*> {
        [[nodiscard]] static constexpr auto stringify( char const* value ) -> std::string {
            return value ? std::string( value ) : std::string("nullptr");
        }
    };
    template<size_t size>
    struct Stringifier<char[size]> {
        [[nodiscard]] static constexpr auto stringify( char const* value ) -> std::string {
            return value;
        }
    };

    template<typename T>
    requires requires{ !std::same_as<std::decay_t<T>, char>; }
    struct Stringifier<T*> {
        [[nodiscard]] static constexpr auto stringify( T* value ) -> std::string {
            return value ? Detail::pointer_to_string( value ) : std::string("nullptr");
        }
    };

    // !TBD: more conversions of built-ins, including containers
    // - also wait for std::format for ranges to match with that

    // Don't specialise this
    template<typename T>
    [[nodiscard]] auto constexpr stringify(T const& value ) {
        if constexpr( std::same_as<T, bool> )
            return value ? "true" : "false";
        else if constexpr( std::is_enum_v<T> )
            return Detail::enum_to_string( value );
        else if constexpr( std::floating_point<T> || std::integral<T> )
            return std::to_string( value );
        else if constexpr( std::is_null_pointer_v<T> )
            return "nullptr";
        else
            return Stringifier<T>::stringify( value );
    }

} // namespace CatchKit

#endif // CATCHKIT_STRINGIFY_H
