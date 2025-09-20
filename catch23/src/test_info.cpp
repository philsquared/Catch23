//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/test_info.h"

#include <algorithm>

namespace CatchKit {
    auto TestInfo::is_manual() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::manual && !tag.ignored; });
    }
    auto TestInfo::should_fail() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::shouldfail && !tag.ignored; });
    }
    auto TestInfo::may_fail() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::mayfail && !tag.ignored; });
    }

}