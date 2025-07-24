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

        // Overload this if your generator has multiple values
        auto size_of(auto const&) { return 100; }

        template<typename G>
        concept IsSingleValueGenerator = requires(G g){ { g.generate() }; };

        template<typename G>
        concept IsMultiValueGenerator = requires(G g, size_t pos){ { g.generate_at(pos) }; };

        // -- generator builders
        // To provide your own generators, either:
        // 1. specialise values_of for your type and generate_value for valuesOf<your type>, or
        // 2. for something more general/ operators on multiple values, specialise size_of and generate_at

        template<typename T> struct values_of {}; // Specialise this for your own types

        template<IsNumeric T>
        struct values_of<T> {
            T from {};
            T up_to = std::numeric_limits<T>::max();

            [[nodiscard]] auto generate() const { return generate_random_number(from, up_to); }
        };

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
            size_t min_len = 0;
            size_t max_len = 65;
            std::string_view charset = Charsets::word_chars; // Must be from string literal

            [[nodiscard]] auto generate() const -> std::string;
        };

        template<typename G>
        auto generate_at(G const& generator, size_t pos) {
            if constexpr( IsSingleValueGenerator<G> )
                return generator.generate();
            else if constexpr( IsMultiValueGenerator<G> )
                return generator.generate_at(pos);
            else
                return generate_value(generator);
        }

        template<typename T>
        struct multiple_values {
            int multiple;
            values_of<T> value_generator;

            auto generate_at(size_t) const { return value_generator.generate(); }
        };

        template<typename T>
        auto size_of(multiple_values<T> const& values) { return values.multiple; }

        template<typename T>
        constexpr auto operator, (int multiple, values_of<T>&& values) {
            return multiple_values<T>{ multiple, std::move(values) };
        }

        template<typename T>
        struct from_values {
            std::vector<T> values;
        };
        template<typename T>
        from_values(std::initializer_list<T> values) -> from_values<T>;

        template<typename T>
        auto size_of(from_values<T> const& seq) { return seq.values.size(); }

        template<typename T>
        auto generate_at(from_values<T> const& seq, size_t pos) { return seq.values[pos]; }


        // Typed generator holder
        template<typename T>
        class GeneratorNode : public ExecutionNode {
            T generator;
            using GeneratedType = decltype(generate_at(generator, 0));
            std::vector<GeneratedType> values;

        public:
            explicit GeneratorNode( NodeId&& id, T&& gen )
            : ExecutionNode(std::move(id), size_of(gen)), generator(std::move(gen)) {
                values.reserve(size);
                for(size_t i=0; i < size; ++i) {
                    values.emplace_back(generate_at(generator, i));
                }
            }

            GeneratedType& current_value() {
                return values[current_index];
            }
            // !TBD shrink?
        };


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

        // Built in matchers
        using Detail::values_of;
        using Detail::from_values;

    } // namespace Generators

} // namespace CatchKit


#define GENERATE(...) \
    [&check]{ using namespace CatchKit::Generators; \
        CatchKit::Detail::GeneratorAcquirer acquirer(check, {#__VA_ARGS__}); \
        if( !acquirer.generator_node ) acquirer.make_generator((__VA_ARGS__)); \
        return acquirer.derived_node<decltype((__VA_ARGS__))>(); \
    }()->current_value()


#endif // CATCH23_GENERATORS_H
