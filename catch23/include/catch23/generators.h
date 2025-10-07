//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_GENERATORS_H
#define CATCH23_GENERATORS_H

#include "generator_node.h"
#include "random.h"

#include <vector>
#include <algorithm>
#include <generator>

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

            template<typename U>
            auto shrink(U&& value) -> std::generator<T> requires IsGeneratorShrinkable<values_of<T>> {
                return value_generator.shrink(std::forward<U>(value));
            }
        };

        template<typename T> requires IsGeneratorShrinkable<values_of<T>>
        struct shrinker_for<multiple_values<T>> : shrinker_for<values_of<T>> {
            template<typename U>
            auto shrink( multiple_values<T>& generator, U&& value ) -> std::generator<T> {
                return shrinker_for<values_of<T>>::shrink( generator.value_generator, std::forward<U>(value) );
            }
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

        };

        template<IsBuiltInNumeric T>
        struct shrinker_for<values_of<T>> {

            enum class Strategies {
                SimpleValues,
                BinaryDescent,
                Ladder1000,
                Ladder100,
                Ladder10,
                Ladder1
            };
            Strategies strategy = Strategies::SimpleValues;

            void rebase() {
                // if( strategy == Strategies::SimpleValues )
                    strategy = Strategies::BinaryDescent;
            }
            auto shrink( values_of<T>& generator, T value ) -> std::generator<T> { // NOSONAR NOLINT (misc-typo)
                if( value >= 0 ) {
                    switch( strategy ) {
                    case Strategies::SimpleValues:
                        co_yield 0;

                        if( value > 1 )
                            co_yield 1;
                        if( value > 2 )
                            co_yield 2;
                        strategy = Strategies::BinaryDescent;
                        [[fallthrough]];
                    case Strategies::BinaryDescent:
                        if( value > 8 )
                            co_yield value/2;
                        strategy = Strategies::Ladder1000;
                        [[fallthrough]];
                    case Strategies::Ladder1000:
                        if( value > 2000 )
                            co_yield value-1000;
                        strategy = Strategies::Ladder100;
                        [[fallthrough]];
                    case Strategies::Ladder100:
                        if( value > 200 )
                            co_yield value-100;
                        strategy = Strategies::Ladder10;
                        [[fallthrough]];
                    case Strategies::Ladder10:
                        if( value > 20 )
                            co_yield value-10;
                        strategy = Strategies::Ladder1;
                        [[fallthrough]];
                    default:
                        if( value > 3 )
                            co_yield --value;
                    }
                }
                else {
                    if constexpr( std::is_signed_v<T> ) {
                        if( strategy == Strategies::SimpleValues ) {
                            co_yield -value;

                            // Explicitly yield a positive 0 for floating point, too
                            if constexpr( std::is_floating_point_v<T> ) // NOSONAR NOLINT (misc-typo)
                                co_yield 0;
                        }
                        // Mirror positive shrinks
                        for(T i: shrink(generator, -value))
                            co_yield -i;
                    }
                }
            }
        };

        template<IsBuiltInNumeric T>
        struct inclusive_range_of {
            T from {};
            T to = std::numeric_limits<T>::max();

            [[nodiscard]] auto generate_at( std::size_t index, RandomNumberGenerator const& ) const { return from + static_cast<T>(index); }
            auto size() const { assert(to > from); return 1 + to - from; }
        };

        // String generator:

        namespace Charsets {
            extern std::string const lcase; // NOLINT (misc-typo)
            extern std::string const ucase; // NOLINT (misc-typo)
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

            auto generate_at( std::size_t pos, RandomNumberGenerator const& ) const { return values[pos]; }
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

        namespace Charsets = Detail::Charsets;

        // Built in matchers
        using Detail::values_of;
        using Detail::from_values;
        using Detail::inclusive_range_of;

    } // namespace Generators

} // namespace CatchKit

#endif // CATCH23_GENERATORS_H
