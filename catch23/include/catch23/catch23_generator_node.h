//
// Created by Phil Nash on 25/07/2025.
//

#ifndef CATCH23_GENERATE_NODE_H
#define CATCH23_GENERATE_NODE_H

#include "catch23_internal_execution_nodes.h"
#include "catch23_test_result_handler.h"

#include "catchkit/catchkit_checker.h"

#include <vector>
#include <memory>

namespace CatchKit {

    namespace Detail {

        template<typename G>
        concept IsSingleValueGenerator = requires(G g){ { g.generate() }; };

        template<typename G>
        concept IsMultiValueGenerator = requires(G g, size_t pos){ { g.generate_at(pos) }; };

        // Overload this if your generator has multiple values
        auto size_of(auto const&) { return 100; }

        template<typename G>
        auto generate_at(G const& generator, size_t pos) {
            if constexpr( IsSingleValueGenerator<G> )
                return generator.generate();
            else if constexpr( IsMultiValueGenerator<G> )
                return generator.generate_at(pos);
            else
                return generate_value(generator);
        }


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

} // namespace CatchKit


#endif //CATCH23_GENERATE_NODE_H
