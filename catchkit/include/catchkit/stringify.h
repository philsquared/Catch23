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
        auto pointer_to_string(void const* p) -> std::string;
        void ignore( auto&& ) noexcept {}

        auto parse_templated_name( std::string const& templated_name, std::string_view function_name ) -> std::string_view;
        auto parse_templated_name( std::string const& templated_name, std::source_location location = std::source_location::current() ) -> std::string_view;
        auto parse_enum_name_from_function(std::string_view function_name, bool fully_qualified = false) -> std::string_view;
        auto unknown_enum_to_string(size_t enum_value) -> std::string;

        constexpr size_t enum_probe_start = 0;
        constexpr size_t enum_probe_end = 16;

        template<typename E, E candidate=static_cast<E>(enum_probe_start), size_t max_probe=enum_probe_end>
        struct enum_value_string {
            static auto find(E e) -> std::string_view {
                if( e == candidate )
                    return parse_enum_name_from_function(std::source_location::current().function_name());
                if constexpr(static_cast<size_t>( candidate) < max_probe )
                    return enum_value_string<E, static_cast<E>(static_cast<size_t>(candidate)+1)>::find(e);
                return {};
            }
        };

        template<typename E>
        auto enum_to_string(E e) -> std::string {
            if( auto val = enum_value_string<E>::find(e); !val.empty() )
                return std::string(val);
            return unknown_enum_to_string(static_cast<size_t>(e));
        }

    }

    #ifdef FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
    template <typename T>
    concept Streamable = requires( std::ostream os, T value ) {
        { os << value };
    };
    #endif

    template<typename T>
    [[nodiscard]] auto constexpr stringify(T const& value );

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

    template<typename T>
    requires requires{ !std::same_as<std::decay_t<T>, char>; }
    struct Stringifier<T*> {
        [[nodiscard]] static constexpr auto stringify( T* value ) -> std::string {
            return value ? Detail::pointer_to_string( value ) : std::string("nullptr");
        }
    };

    template<typename T, typename A>
    struct Stringifier<std::vector<T, A>> {
        [[nodiscard]] static constexpr auto stringify( std::vector<T, A> const& values ) -> std::string {
            std::string s = "[";
            bool first = true;
            for(auto&& val : values) {
                if( !first )
                    s += ", ";
                else
                    first = false;
                s += CatchKit::stringify( val );
            }
            s += "]";
            return s;
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
        else if constexpr( std::is_convertible_v<T, std::string> )
            return std::format("\"{}\"", value);
        else
            return Stringifier<T>::stringify( value );
    }

} // namespace CatchKit

#endif // CATCHKIT_STRINGIFY_H
