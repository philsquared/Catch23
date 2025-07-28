//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_ASSERTION_CONTEXT_H
#define CATCHKIT_ASSERTION_CONTEXT_H

#include <source_location>
#include <string_view>

namespace CatchKit::Detail {

    struct AssertionContext {
        std::string_view macro_name;
        std::string_view original_expression;
        std::string_view message  = {};
        std::source_location location = std::source_location::current();

        auto combine_messages(std::string_view additional_message) -> std::string;
    };

} // namespace CatchKit::Detail

namespace CatchKit {

    using Detail::AssertionContext;

} // namespace CatchKit

#endif // CATCHKIT_ASSERTION_CONTEXT_H
