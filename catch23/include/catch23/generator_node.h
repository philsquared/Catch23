//
// Created by Phil Nash on 25/07/2025.
//

#ifndef CATCH23_GENERATE_NODE_H
#define CATCH23_GENERATE_NODE_H

#include "internal_execution_nodes.h"
#include "test_result_handler.h"
#include "random.h"

#include "catchkit/checker.h"

#include <vector>
#include <memory>

namespace CatchKit::Detail {

    class RandomNumberGenerator;

    template<typename G>
    concept IsSingleValueGenerator = requires(G g, RandomNumberGenerator& rng){ { g.generate(rng) }; };

    template<typename G>
    concept IsMultiValueGenerator = requires(G const& g, std::size_t pos, RandomNumberGenerator& rng){ { g.generate_at(pos, rng) }; };

    template<typename G>
    concept IsSizedGenerator = requires(G const& g, std::size_t pos){ { g.size(pos) } -> std::same_as<std::size_t>; };

    template<typename G>
    auto size_of(G const& generator, std::size_t default_size = 100) {
        if constexpr( IsMultiValueGenerator<G> ) {
            static_assert( !IsSizedGenerator<G>, "Generator has generate_at() but not size()");
            return generator.size();
        }
        else
            return default_size;
    }

    template<typename G>
    auto generate_at(G const& generator, std::size_t pos, RandomNumberGenerator& rng) {
        if constexpr( IsSingleValueGenerator<G> )
            return generator.generate(rng);
        else if constexpr( IsMultiValueGenerator<G> )
            return generator.generate_at(pos, rng);
        else
            static_assert(false, "not a generator");
    }

    constexpr std::size_t default_repetitions = 100; // Make this runtime configurable?

    auto dummy_random_number_generator() -> RandomNumberGenerator&;

    // Typed generator holder node
    template<typename T>
    class GeneratorNode : public ExecutionNode {
        T generator;
        using GeneratedType = decltype(generate_at(generator, 0, dummy_random_number_generator()));
        std::vector<GeneratedType> values;
        std::size_t size;
    public:
        explicit GeneratorNode( NodeId&& id, T&& gen )
        :   ExecutionNode(std::move(id)),
            generator(std::move(gen)),
            size(size_of(generator, default_repetitions))
        {
            values.reserve(size);
            RandomNumberGenerator rng;
            for(std::size_t i=0; i < size; ++i) {
                values.emplace_back(generate_at(generator, i, rng));
            }
        }

        auto move_next() -> bool override {
            return ++current_index == size;
        }

        GeneratedType& current_value() {
            assert(current_index < values.size());
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

} // namespace CatchKit::Detail


#endif // CATCH23_GENERATE_NODE_H
