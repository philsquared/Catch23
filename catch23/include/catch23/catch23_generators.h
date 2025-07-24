//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23_internal_execution_nodes.h"
#include "catch23_test_result_handler.h"
#include "catch23_random.h"

#include "catchkit/catchkit_checker.h"

#include <vector>
#include <source_location>
#include <memory>

#ifndef CATCH23_GENERATORS_H
#define CATCH23_GENERATORS_H

namespace CatchKit {

    namespace Detail {

        // Typed generator holder
        template<typename T>
        class GeneratorNode : public ExecutionNode {
            using ValueType = T::ValueType;
            T generator;
            std::vector<ValueType> values;

        public:
            explicit GeneratorNode( NodeId&& id, T&& gen )
            : ExecutionNode(std::move(id), size_of(gen)), generator(std::move(gen)) {
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

        // -- generator builders
        // To provide your own generators, either:
        // 1. specialise values_of for your type and generate_value for valuesOf<your type>, or
        // 2. for something more general/ operators on multiple values, specialise size_of and generate_at
        // In any case they need an embedded ValueType

        template<typename T> constexpr T min_of;
        template<typename T> constexpr T max_of;

        template<std::integral T> constexpr T min_of<T> = std::numeric_limits<T>::min();
        template<std::integral T> constexpr T max_of<T> = std::numeric_limits<T>::max();

        template<typename T>
        struct values_of {
            using ValueType = T;
            T from{};
            T up_to = max_of<T>;
        };
        template<typename T>
        auto size_of(values_of<T> const&) { return 1; }

        template<std::integral T>
        auto generate_value(values_of<T> const& generator) {
            return generate_random_number(generator.from, generator.up_to);
        }

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
            using ValueType = std::string;
            size_t min_len = 0;
            size_t max_len = 65;
            std::string_view charset = Charsets::word_chars; // Must be from string literal
        };
        auto generate_value(values_of<std::string> const&) -> std::string;

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
        constexpr auto operator, (int multiple, values_of<T>&& values) {
            return multiple_values{multiple, std::move(values) };
        }

        struct GeneratorAcquirer {
            ExecutionNodes& execution_nodes;
            NodeId id;
            ExecutionNode* generator_node;

            GeneratorAcquirer(Checker& checker, NodeId&& id)
                :   execution_nodes(get_execution_nodes_from_result_handler(checker.result_handler)),
                    id(std::move(id)),
                    generator_node(execution_nodes.find_node(this->id))
            {}

            template<typename T>
            void make_generator(T&& gen) {
                generator_node = &execution_nodes.add_node( std::make_unique<GeneratorNode<T>>(std::move(id), std::forward<T>(gen)) );
            }
            template<typename T>
            auto derived_node() {
                assert(generator_node != nullptr);
                generator_node->enter();
                return static_cast<GeneratorNode<T>*>(generator_node);
            }
        };

    } // namespace Detail

    namespace Generators {

        namespace Charsets { using namespace Detail::Charsets; }
        using Detail::values_of;

    } // namespace Generators

} // namespace CatchKit


#define GENERATE(...) \
        [&check]{ using namespace CatchKit::Generators; \
            CatchKit::Detail::GeneratorAcquirer acquirer(check, {#__VA_ARGS__}); \
            if( !acquirer.generator_node ) acquirer.make_generator((__VA_ARGS__)); \
            return acquirer.derived_node<decltype((__VA_ARGS__))>(); \
        }()->current_value()


#endif // CATCH23_GENERATORS_H
