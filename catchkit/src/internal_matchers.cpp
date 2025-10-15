//
// Created by Phil Nash on 30/07/2025.
//

#include "catchkit/internal_matchers.h"

namespace CatchKit::Detail {

    void MatchResult::set_address( uintptr_t address ) {
        assert(matcher_address == 0 || matcher_address == address);
        matcher_address = address;
    }

    auto CompositeMatchResult::add_children_from(CompositeMatchResult const& other) -> CompositeMatchResult&& {
        child_results.reserve( child_results.size() + other.child_results.size() );
        std::ranges::copy( other.child_results, std::back_inserter( child_results ) );
        return std::move(*this);
    }

    auto CompositeMatchResult::make_child_of(uintptr_t address) -> CompositeMatchResult&& {
        child_results.emplace_back( result, std::exchange( matcher_address, address ) );
        return std::move(*this);
    }

} // namespace CatchKit::Detail