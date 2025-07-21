//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/catchkit_checker.h"
#include "catchkit/catchkit_assert_result_handler.h"

namespace {
    CatchKit::Detail::AssertResultHandler default_assertion_handler;
    constinit CatchKit::Detail::ResultHandler* current_assertion_handler = &default_assertion_handler;
}

constinit CatchKit::Checker check(default_assertion_handler, CatchKit::ResultDisposition::Continue);
constinit CatchKit::Checker require(default_assertion_handler, CatchKit::ResultDisposition::Abort);

namespace CatchKit::Detail {

    // !TBD Do we need to return the old one so it can be set back later?
    // This would probably change to being a pointer
    void set_current_assertion_handler( AssertResultHandler& handler ) {
        current_assertion_handler = &handler;
    }

    auto Checker::operator()(std::string_view message, std::source_location assertion_location) -> Checker& {
        return operator()(AssertionContext{{}, {}, message, assertion_location});
    }

    auto Checker::operator()(AssertionContext const& context) -> Checker& {
        current_context = context;
        result_handler.on_assertion_start(current_context);
        return *this;
    }

    auto Checker::combine_messages( std::string_view additional_message ) -> std::string {
        if(!current_context.message.empty()) {
            if(!additional_message.empty())
                return std::string(current_context.message) + "\n" + std::string(additional_message);
            return std::string(current_context.message);
        }
        if(!additional_message.empty())
            return std::string(additional_message);
        return {};
    }

} // namespace CatchKit::Detail
