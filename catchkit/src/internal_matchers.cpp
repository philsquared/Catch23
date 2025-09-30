//
// Created by Phil Nash on 30/07/2025.
//

#include "catchkit/internal_matchers.h"

namespace CatchKit::Detail {

    auto MatchResult::make_child_of(uintptr_t address) -> MatchResult& {
        child_results.emplace_back( result, std::exchange( matcher_address, address ) );
        return *this;
    }

    auto MatchResult::set_address(uintptr_t address) -> MatchResult& {
        assert(matcher_address == 0 || matcher_address == address);
        matcher_address = address;
        return *this;
    }

    auto MatchResult::add_children_from(MatchResult const& other) -> MatchResult& {
        child_results.reserve( child_results.size() + other.child_results.size() );
        std::ranges::copy( other.child_results, std::back_inserter( child_results ) );
        return *this;
    }

} //namespace CatchKit::Detail