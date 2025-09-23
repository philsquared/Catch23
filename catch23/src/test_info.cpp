//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/test_info.h"

#include <algorithm>

namespace CatchKit {

    auto TestInfo::has_tag_type(Tag::Type tag_type) const -> bool {
        return std::ranges::any_of(tags, [tag_type](auto const& tag) { return tag.type == tag_type && !tag.ignored; });
    }
    auto TestInfo::should_fail() const -> bool {
        return has_tag_type(Tag::Type::should_fail);
    }
    auto TestInfo::may_fail() const -> bool {
        return has_tag_type(Tag::Type::may_fail);
    }
}