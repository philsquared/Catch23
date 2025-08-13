//
// Created by Phil Nash on 25/07/2025.
//

#ifndef CATCH23_GENERATE_NODE_H
#define CATCH23_GENERATE_NODE_H

#include <generator>

#include "internal_execution_nodes.h"
#include "test_result_handler.h"
#include "random.h"

#include "catchkit/checker.h"

#include <vector>
#include <memory>
#include <set>
#include <print> // !DBG

namespace CatchKit::Detail {

    template<typename G>
    struct shrinker_for;

    template<typename G>
    concept IsSingleValueGenerator = requires(G g, RandomNumberGenerator& rng){ { g.generate(rng) }; };

    template<typename G>
    concept IsMultiValueGenerator = requires(G const& g, std::size_t pos, RandomNumberGenerator& rng){ { g.generate_at(pos, rng) }; };

    template<typename G>
    concept IsGeneratorSized = requires(G const& g, std::size_t pos){ { g.size(pos) } -> std::same_as<std::size_t>; };

    template<typename G>
    auto size_of(G const& generator, std::size_t default_size = 100) {
        if constexpr( IsMultiValueGenerator<G> ) {
            static_assert( !IsGeneratorSized<G>, "Generator has generate_at() but not size()");
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

    auto make_dummy_rng() -> RandomNumberGenerator&;

    template<typename GeneratorType>
    using get_generated_type = decltype(generate_at(std::declval<GeneratorType>(), 0, make_dummy_rng()));

    template<typename G>
    concept IsGeneratorShrinkable = requires(G& g, get_generated_type<G> val){ { shrinker_for<G>().shrink(g, val) }; };


    // This is a dummy implementation that saves us having to constexpr guard calls that aren't taken
    template<typename GeneratorType, typename GeneratedType>
    struct Shrinker {
        GeneratedType original_failing_value;
        explicit Shrinker(GeneratorType const&, GeneratedType const&, std::set<GeneratedType>&) {}
        [[nodiscard]] auto shrink() const -> bool { return false; }
    };

    template<IsGeneratorShrinkable GeneratorType, typename GeneratedType>
    struct Shrinker<GeneratorType, GeneratedType> {
        GeneratedType& current_value;
        GeneratedType original_failing_value;
        shrinker_for<GeneratorType> shrinker;
        std::generator<GeneratedType> shrink_generator;
        using iterator = decltype(shrink_generator.begin());
        iterator it;
        std::set<GeneratedType>& cache;

        Shrinker(GeneratorType& generator, GeneratedType& current_value, std::set<GeneratedType>& cache)
        :   current_value(current_value),
            original_failing_value(current_value),
            shrink_generator( shrinker.shrink( generator, original_failing_value ) ),
            it( shrink_generator.begin() ),
            cache(cache)
        {}

        [[nodiscard]] auto shrink() -> bool {
            while( it != shrink_generator.end() ) {
                current_value = *it;
                ++it;
                if( !cache.contains( current_value ) ) {
                    if( cache.size() < 10 )
                        cache.insert( current_value );
                    std::print("trying: {}", current_value);
                    return true;
                }
            }
            return false;
        }
    };


    // Typed generator holder node
    template<typename GeneratorType>
    class GeneratorNode : public ExecutionNode, public ShrinkableNode {
        GeneratorType generator;
        RandomNumberGenerator rng;
        std::size_t size;
        using GeneratedType = get_generated_type<GeneratorType>;
        GeneratedType current_generated_value;
        std::optional<GeneratedType> pre_shrunk_value;
        std::optional<Shrinker<GeneratorType, GeneratedType>> shrinker;
        std::set<GeneratedType> cache;
    public:
        explicit GeneratorNode( NodeId&& id, GeneratorType&& gen )
        :   ExecutionNode(std::move(id)),
            generator(std::move(gen)),
            size(size_of(generator, default_repetitions)),
            current_generated_value( generate_value() )
        {
            if( IsGeneratorShrinkable<GeneratorType> ) {
                shrinkable = this;
            }
        }

        auto generate_value() {
            return generate_at(generator, current_index, rng);
        }
        void move_first() override {
            assert( !shrinker );
            rng.reset();
            current_generated_value = generate_value();
        }
        auto move_next() -> bool override {
            assert( !shrinker );
            if( ++current_index == size )
                return true;
            current_generated_value = generate_value();
            return false;
        }

        GeneratedType& current_value() {
            return current_generated_value;
        }

        // ShrinkableNode interface:
        void start_shrinking() override {
            pre_shrunk_value = current_generated_value;
            shrinker.emplace( generator, current_generated_value, cache );
        }
        void rebase_shrink() override {
            assert( shrinker );
            shrinker.emplace( generator, current_generated_value, cache );
        }

        auto shrink() -> bool override {
            assert(shrinker);
            return shrinker->shrink();
        }

        auto stop_shrinking() -> bool override {
            assert(shrinker);
            current_generated_value = shrinker->original_failing_value;
            bool shrunk = (pre_shrunk_value != current_generated_value);
            shrinker.reset();
            pre_shrunk_value.reset();
            current_index = size-1;
            return shrunk;

        }
        auto current_value_as_string() -> std::string override {
            return stringify(current_generated_value);
        }
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
