//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_RUNNER_H
#define CATCH23_RUNNER_H

#include "print.h"
#include "test_registry.h"
#include "test.h"
#include "reporter.h"
#include "test_result_handler.h"

namespace CatchKit::Detail {

    template<typename R, typename T>
    concept range_of = std::ranges::range<R> &&
                       std::same_as<std::ranges::range_value_t<R>, T>;

    class TestRunner {
        TestResultHandler result_handler;
    public:
        explicit TestRunner( Reporter& reporter ) : result_handler(reporter) {}

        void run_test( Test const& test );

        void run_tests( TestRegistry const& tests );

        void run_tests( range_of<Test> auto const& tests ) {
            result_handler.get_reporter().on_test_run_start();

            std::vector<Test const*> tests_to_run;
            bool soloing = false;
            for( auto&& test : tests) {
                if( test.test_info.has_tag_type(Tag::Type::solo) ) {
                    if( !soloing ) {
                        tests_to_run.clear();
                        soloing = true;
                    }
                    tests_to_run.push_back( &test );
                }
                else if( !soloing && !test.test_info.has_tag_type(Tag::Type::mute) ) {
                    tests_to_run.push_back( &test );
                }
            }
            if( soloing )
                println( ColourIntent::Warning, "\nWarning: Running soloed test(s) (tests with the [solo] tag) only.\n");
            for( auto const test : tests_to_run) {
                run_test( *test );
            }

            result_handler.get_reporter().on_test_run_end();
        }
    };

} // namespace CatchKit::Detail

namespace CatchKit {
    using Detail::TestRunner;
}

#endif // CATCH23_RUNNER_H
