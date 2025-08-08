//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_GENERATORS_H
#define CATCH23_GENERATORS_H

#include "generator_node.h"
#include "random.h"

#include <vector>
#include <source_location>
#include <algorithm>

namespace CatchKit {

    namespace Detail {

        // To provide your own generators, either:
        // 1. specialise values_of for your type and generate_value for valuesOf<your type>, or
        // 2. for something more general/ operators on multiple values, specialise size_of and generate_at

        template<typename T> struct values_of {}; // Specialise this for your own types


        // Adapter to specify number of repetitions:

        template<typename T>
        struct multiple_values {
            std::size_t multiple;
            values_of<T> value_generator;

            auto generate_at( std::size_t, RandomNumberGenerator& rng ) const { return value_generator.generate(rng); }
            auto size() const { return multiple; }
        };


        template<typename T>
        constexpr auto operator, ( std::size_t multiple, values_of<T>&& values ) {
            return multiple_values<T>{ multiple, std::move(values) };
        }


        // Numeric (int or real) generators:

        template<IsBuiltInNumeric T>
        struct values_of<T> {
            T from {};
            T up_to = std::numeric_limits<T>::max();

            [[nodiscard]] auto generate( RandomNumberGenerator& rng ) const { return rng.generate(from, up_to); }

            // Given a starting value, try different strategies to produce a simpler value that might
            // also fail. If it still passes this function will be called again with an incremented
            // strategy number, and this will repeat until either a failure is found
            // or the function returns an empty optional

            // Could use coroutines to "simplify" this?
            auto shrink(T value, int strategy_index) -> std::optional<T>;
        };

        template<IsBuiltInNumeric T>
        struct inclusive_range_of {
            T from {};
            T to = std::numeric_limits<T>::max();

            [[nodiscard]] auto generate_at( std::size_t index, RandomNumberGenerator& ) const { return from + static_cast<T>(index); }
            auto size() const { assert(to > from); return 1 + to - from; }
        };

        // String generator:

        namespace Charsets {
            extern std::string const lcase;
            extern std::string const ucase;
            extern std::string const all_alpha;
            extern std::string const numbers;
            extern std::string const alphanumeric;
            extern std::string const word_chars;
            extern std::string const symbols;
            extern std::string const printable_ascii;
        }

        template<>
        struct values_of<std::string> {
            std::size_t min_len = 0;
            std::size_t max_len = 65;
            std::string_view charset = Charsets::word_chars; // Must be from string literal

            [[nodiscard]] auto generate( RandomNumberGenerator& rng ) const -> std::string;
        };


        // Generate specific values:

        template<typename T>
        struct from_values {
            std::vector<T> values;

            auto generate_at( std::size_t pos, RandomNumberGenerator& ) const { return values[pos]; }
            auto size() const { return values.size(); }

        };
        template<typename T>
        from_values(std::initializer_list<T> values) -> from_values<T>;

        // Vector generators

        template<typename T>
        struct values_of<std::vector<T>> {
            std::size_t min_size = 0;
            std::size_t max_size = 65;
            values_of<T> value_generator;

            [[nodiscard]] auto generate( RandomNumberGenerator& rng ) const {
                std::vector<T> values( rng.generate( min_size, max_size ) );
                std::ranges::generate(values, [generator=value_generator, &rng]{ return generator.generate(rng); });
                return values;
            }
        };

    } // namespace Detail

    namespace Generators {

        namespace Charsets { using namespace Detail::Charsets; }

        // Built in matchers
        using Detail::values_of;
        using Detail::from_values;
        using Detail::inclusive_range_of;

    } // namespace Generators

} // namespace CatchKit


#define GENERATE(...) \
    [&check]{ using namespace CatchKit::Generators; \
        CatchKit::Detail::GeneratorAcquirer acquirer(check, {#__VA_ARGS__}); \
        if( !acquirer.generator_node ) acquirer.make_generator((__VA_ARGS__)); \
        return acquirer.derived_node<decltype((__VA_ARGS__))>(); \
    }()->current_value()


#endif // CATCH23_GENERATORS_H
