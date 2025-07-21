//
// Created by Phil Nash on 22/07/2025.
//

#include <vector>
#include <source_location>
#include <memory>

#ifndef CATCH23_GENERATORS_H
#define CATCH23_GENERATORS_H

namespace CatchKit::Generators {

    namespace Detail {

        // Base class for polymorphic generator holder
        class Generator {
        protected:
            size_t size;
            size_t current_index = 0;

            explicit Generator(size_t size) : size(size) {}

        public:
            virtual ~Generator() = default;
            bool move_next();
        };

        // Typed generator holder
        template<typename T>
        class GeneratorHolder : public Generator {
            using ValueType = T::ValueType;
            T generator;
            std::vector<ValueType> values;

        public:
            explicit GeneratorHolder( T&& gen) : Generator(size_of(gen)), generator(std::move(gen)) {
                values.reserve(size);
                for(size_t i=0; i < size; ++i) {
                    values.emplace_back(generate_at(generator, i));
                }
            }

            ValueType& current_value() {
                return values[current_index];
            }
            // !TBD shrink?
        };

        auto add_generator(std::unique_ptr<Generator> generator, std::source_location loc=std::source_location::current()) -> Generator*;
        auto find_generator(std::source_location loc=std::source_location::current()) -> Generator*;

        template<typename T>
        auto make_generator(T&& gen, std::source_location loc=std::source_location::current()) -> Generator* {
            return add_generator(std::make_unique<GeneratorHolder<T>>(std::forward<T>(gen)), loc);
        }

        // -- generator builders
        // To provide your own generators, either:
        // 1. specialise values_of for your type and generate_value for valuesOf<your type>, or
        // 2. for something more general/ operators on multiple values, specialise size_of and generate_at

        template<typename T> constexpr T min_of;
        template<typename T> constexpr T max_of;

        template<std::integral T> constexpr T min_of<T> = std::numeric_limits<T>::min();
        template<std::integral T> constexpr T max_of<T> = std::numeric_limits<T>::max();

        template<typename T>
        struct values_of {
            using ValueType = T;
            T from = min_of<T>;
            T to = max_of<T>;
        };
        template<typename T>
        auto size_of(values_of<T> const&) { return 1; }

        template<std::integral T>
        auto generate_value(values_of<T> const&) {
            static int i = 0;
            return i++;
            // !TBD generate random
        }


        template<>
        struct values_of<std::string> {
            using ValueType = std::string;
            size_t min_len = 0;
            size_t max_len = 65;
        };
        inline auto generate_value(values_of<std::string> const&) {
            return "<random>"; // !TBD generate random
        }

        template<typename T>
        auto generate_at(values_of<T> const& values, size_t) {
            return generate_value(values);
        }

        template<typename T>
        struct multiple_values {
            using ValueType = T;
            int multiple;
            values_of<T> value_generator;
        };

        template<typename T>
        auto size_of(multiple_values<T> const& values) { return values.multiple; }

        template<typename T>
        auto generate_at(multiple_values<T> const& values, size_t) { return generate_value(values.value_generator); }

        template<typename T>
        constexpr auto operator*(int multiple, values_of<T>&& values) {
            return multiple_values{multiple, std::move(values) };
        }

    } // namespace Detail

    using Detail::values_of;

} // namespace CatchKit::Generators

#define GENERATE(...) \
        []{ using namespace CatchKit::Generators; \
            auto gen = Detail::find_generator(); \
            if( !gen ) gen = Detail::make_generator(__VA_ARGS__); \
            return static_cast<Detail::GeneratorHolder<decltype(__VA_ARGS__)>*>(gen); }() \
    ->current_value()

#endif //CATCH23_GENERATORS_H
