//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/catchkit_assertion_context.h"

#include <string>

namespace CatchKit::Detail {

    auto AssertionContext::combine_messages( std::string_view additional_message ) -> std::string {
        if(!message.empty()) {
            if(!additional_message.empty())
                return std::string(message) + "\n" + std::string(additional_message);
            return std::string(message);
        }
        if(!additional_message.empty())
            return std::string(additional_message);
        return {};
    }

} // namespace CatchKit::Detail