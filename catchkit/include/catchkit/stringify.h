//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_STRINGIFY_H
#define CATCHKIT_STRINGIFY_H

#include <string>
#include <string_view>
#include <source_location>
#include <format>

// #define CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS

#ifdef CATCHKIT_FALLBACK_TO_OSTREAM_STRING_CONVERSIONS
#include <sstream>
#endif

namespace CatchKit {
    namespace Detail {
        auto pointer_to_string(void const* p) -> std::string;
        void ignore( auto&& ) noexcept { /* this function doesn't do anything */ }

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
struct std::formatter<T> {
    constexpr auto parse( std::format_parse_context& ctx ) { return ctx.begin(); }
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
            return Detail::enum_to_string( value );
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
            std::ostringstream oss; // !TBD: use an ostringstream pool?
            oss << value;
            return oss.str();
        }
#endif
        else
            return "{?}";
    }

} // namespace CatchKit

#endif // CATCHKIT_STRINGIFY_H
