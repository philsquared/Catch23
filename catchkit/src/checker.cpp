//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/checker.h"
#include "catchkit/assert_result_handler.h"

namespace {
    CatchKit::Detail::AssertResultHandler default_assertion_handler; // NOSONAR NOLINT (misc-typo)
}

constinit CatchKit::Checker checker{ &default_assertion_handler }; // NOSONAR NOLINT (misc-typo)

namespace CatchKit::Detail {

    auto Checker::check(AssertionContext const& context, InvertResult invert_result) -> Asserter {
        result_handler->on_assertion_start(ResultDisposition::Continue, context);
        return Asserter( *this, invert_result );
    }
    auto Checker::require(AssertionContext const& context, InvertResult invert_result) -> Asserter {
        result_handler->on_assertion_start(ResultDisposition::Abort, context);
        return Asserter( *this, invert_result );
    }

    Asserter::Asserter( Checker& checker, InvertResult invert_result ) : checker(checker), invert_result(invert_result) {
        if( checker.message_stream )
            checker.message_stream->clear();
    }
    Asserter::~Asserter() noexcept(false) {
        if( expression_info ) {
            if( checker.message_stream )
                checker.result_handler->on_assertion_result_detail( *expression_info, checker.message_stream->str() );
            else
                checker.result_handler->on_assertion_result_detail( *expression_info, {} );
        }

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
