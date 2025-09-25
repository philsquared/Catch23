//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_STRINGIFY_H
#define CATCHKIT_STRINGIFY_H

#include "reflection.h"

#include <string>
#include <string_view>
#include <format>

// #define CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS

#ifdef CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
#include <sstream>
#endif

namespace CatchKit {
    // Specialise this with a stringify member function for your own conversions
    template<typename T>
    struct Stringifier;

    template<typename T>
    concept Stringifiable = requires { Stringifier<T>::stringify; };

    template<typename T>
    [[nodiscard]] auto constexpr stringify(T const& value );
}

// This allows any type for which there is a Stringifier specialisation to be usable by std::format.
// This is necessary so that ranges/ containers of custom types will work.
// Note that if you provide both a std::formatter _and_ a Stringifier for your types,
// then stringified _containers_ (or ranges) of them will prefer the std::formatter specialisations.
template<CatchKit::Stringifiable T>
struct std::formatter<T> { // NOLINT
    constexpr static auto parse( std::format_parse_context const& ctx ) { return ctx.begin(); }
    auto format(T const& val, auto& ctx) const {
        return std::format_to( ctx.out(), "{}", CatchKit::Stringifier<T>::stringify(val) );
    }
};

namespace CatchKit {
#ifdef FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
    template <typename T>
    concept Streamable = requires( std::ostream os, T value ) {
        { os << value };
    };
#endif

    // Don't specialise this, specialise Stringifier instead
    template<typename T>
    [[nodiscard]] auto constexpr stringify(T const& value ) {
        if constexpr( std::is_enum_v<T> )
            return enum_to_string( value );
        else if constexpr( std::is_null_pointer_v<T> )
            return "nullptr";
        else if constexpr( std::is_convertible_v<T, char const*> && std::is_pointer_v<T> )
            return value ? std::format("\"{}\"", value) : std::string("nullptr");
        else if constexpr( std::is_convertible_v<T, std::string> )
            return std::format("\"{}\"", value);
        else if constexpr( std::formattable<T, char> )
            return std::format("{}", value);
#ifdef FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
        else if constexpr ( Streamable<T> ) {
            std::ostringstream oss;
            oss << value;
            return oss.str();
        }
#endif
        else
            return "{?}";
    }

} // namespace CatchKit

#endif // CATCHKIT_STRINGIFY_H
