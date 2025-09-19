//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/test_info.h"

#include <algorithm>

namespace CatchKit {
    auto TestInfo::is_manual() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.ignored != (tag.type == Tag::Type::manual); });
    }
    auto TestInfo::should_fail() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.ignored != (tag.type == Tag::Type::shouldfail); });
    }
    auto TestInfo::may_fail() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.ignored != (tag.type == Tag::Type::mayfail); });
    }

}