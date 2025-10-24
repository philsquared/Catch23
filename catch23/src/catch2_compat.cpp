//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/catch2_compat.h"

namespace CatchKit::Detail {
    auto make_test_info(std::source_location location, std::string name, std::string_view tag_spec ) -> TestInfo {

        // Parse tags out of the tag string
        std::vector<Tag> tags;
        while(true) {
            auto start = tag_spec.find('[');
            if( start == std::string::npos )
                break;
            auto end = tag_spec.find(']', start);
            if( end == std::string::npos )
                throw std::invalid_argument("Invalid tag specification - missing an expected `]`");
            std::string tag_name(tag_spec.substr( start+1, end-start-1 ));
            if( tag_name == "!mayfail" ) // NOLINT (misc-typo)
                tags.emplace_back(Tags::may_fail);
            if( tag_name == "!shouldfail" ) // NOLINT (misc-typo)
                tags.emplace_back(Tags::should_fail);
            else if( tag_name.starts_with( "." ) )
                tags.emplace_back(tag_name, Tag::Type::mute);
            else
                tags.emplace_back(std::move(tag_name));
            tag_spec = tag_spec.substr(end);
        }

        return { location, std::move(name), std::move(tags) };
    }

} // namespace CatchKit::Detail
