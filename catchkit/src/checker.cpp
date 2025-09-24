//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/checker.h"
#include "catchkit/assert_result_handler.h"

namespace {
    CatchKit::Detail::AssertResultHandler default_assertion_handler; // NOSONAR
}

constinit CatchKit::Checker check{ &default_assertion_handler, CatchKit::ResultDisposition::Continue }; // NOSONAR
constinit CatchKit::Checker require{ &default_assertion_handler, CatchKit::ResultDisposition::Abort }; // NOSONAR

namespace CatchKit::Detail {

    auto Checker::operator()(std::string_view message, std::source_location assertion_location) -> Asserter {
        return operator()(AssertionContext{{}, {}, message, assertion_location});
    }

    auto Checker::operator()(AssertionContext const& context) -> Asserter {
        result_handler->on_assertion_start(result_disposition, context);
        return Asserter{*this};
    }

    Asserter::~Asserter() noexcept(false) {
        checker.result_handler->on_assertion_end(); // This may throw to cancel the test
    }

    void Asserter::report_current_exception() const {
        if( checker.result_handler->on_assertion_result(ResultType::Failed) == ResultDetailNeeded::Yes ) {
            checker.result_handler->on_assertion_result_detail(
                ExceptionExpressionInfo{
                    get_exception_message(
                        std::current_exception()),
                        ExceptionExpressionInfo::Type::Unexpected },
                    {} );
        }
    }


} // namespace CatchKit::Detail
