//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/test_info.h"

#include <algorithm>

namespace CatchKit {
    auto TestInfo::is_muted() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::mute && !tag.ignored; });
    }
    auto TestInfo::is_soloed() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::solo && !tag.ignored; });
    }
    auto TestInfo::should_fail() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::shouldfail && !tag.ignored; });
    }
    auto TestInfo::may_fail() const -> bool {
        return std::ranges::any_of(tags, [](auto const& tag) { return tag.type == Tag::Type::mayfail && !tag.ignored; });
    }

}