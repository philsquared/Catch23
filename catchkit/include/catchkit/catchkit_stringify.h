//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_STRINGIFY_H
#define CATCHKIT_STRINGIFY_H

#include <string>
#include <string_view>

#define CATCHKIT_FALLBACK_TO_FORMAT_STRING_CONVERSIONS
// #define CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS

#ifdef CATCHKIT_FALLBACK_TO_FORMAT_STRING_CONVERSIONS
#include <format>
#endif
#ifdef CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
#include <sstream>
#endif

namespace CatchKit {

    void ignore( auto&& ) noexcept {}

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
            ignore( value );
            #endif

            return "{?}";
        }
    };

    template<>
    struct Stringifier<bool> {
        [[nodiscard]] static constexpr auto stringify( bool value ) -> std::string {
            return value ? "true" : "false";
        }
    };

    // to_string is a good balance of compile-time and run-time performance, without external dependencies
    // in the case where you need to end up with a std::string, anyway
    // (faster algos are typically lower-level and need to be followed up with a copy)
    // see: https://www.zverovich.net/2020/06/13/fast-int-to-string-revisited.html
    template<std::integral T>
    struct Stringifier<T> {
        [[nodiscard]] static constexpr auto stringify( T value ) -> std::string { return std::to_string( value ); }
    };

    template<std::floating_point T>
    struct Stringifier<T> {
        // !TBD: customise output (e.g. strip trailing zeros)?
        [[nodiscard]] static constexpr auto stringify( T value ) -> std::string { return std::to_string( value ); }
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
            return value;
        }
    };
    template<size_t size>
    struct Stringifier<char[size]> {
        [[nodiscard]] static constexpr auto stringify( char const* value ) -> std::string {
            return { value, size };
        }
    };
    // !TBD: more conversions of built-ins, including containers
    // - also wait for std::format for ranges to match with that

    // Don't specialise this
    template<typename T>
    [[nodiscard]] auto constexpr stringify(T const& value ) {
        return Stringifier<T>::stringify( value );
    }

} // namespace CatchKit

#endif // CATCHKIT_STRINGIFY_H
