//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/checker.h"
#include "catchkit/assert_result_handler.h"

namespace {
    CatchKit::Detail::AssertResultHandler default_assertion_handler;
    constinit CatchKit::Detail::ResultHandler* current_assertion_handler = &default_assertion_handler;
}

constinit CatchKit::Checker check( &default_assertion_handler, CatchKit::ResultDisposition::Continue );
constinit CatchKit::Checker require( &default_assertion_handler, CatchKit::ResultDisposition::Abort );

namespace CatchKit::Detail {

    // !TBD Do we need to return the old one so it can be set back later?
    // This would probably change to being a pointer
    void set_current_assertion_handler( AssertResultHandler& handler ) {
        current_assertion_handler = &handler;
    }

    auto Checker::operator()(std::string_view message, std::source_location assertion_location) -> Asserter {
        return operator()(AssertionContext{{}, {}, message, assertion_location});
    }

    auto Checker::operator()(AssertionContext&& context) -> Asserter {
        result_handler->on_assertion_start(result_disposition, std::move(context));
        return Asserter{*this};
    }

} // namespace CatchKit::Detail
