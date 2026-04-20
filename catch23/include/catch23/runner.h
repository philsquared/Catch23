//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_RUNNER_H
#define CATCH23_RUNNER_H

#include <algorithm>

#include "config.h"
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
        Config config;

        void run_tests( std::vector<Test const*> const& tests_to_run, bool soloing );

    public:
        explicit TestRunner( Reporter& reporter, Config config )
        :   result_handler(reporter),
            config(std::move(config))
        {}

        void run_test( Test const& test );

        void run_tests( TestRegistry const& tests );

        void run_tests( range_of<Test> auto& tests ) {
            static auto to_ptr = std::views::transform(
                []( Test const& test ) { return &test; });

            auto soloed = tests
                | std::views::filter( []( Test const& test ) {
                        return test.test_info.has_tag_type(Tag::Type::solo);
                    })
                | to_ptr
                | std::ranges::to<std::vector>();
            if( !soloed.empty() )
                return run_tests( soloed, true );

            auto not_muted = tests
                | std::views::filter([this]( Test const& test ) { return should_test_run(test);})
                | to_ptr
                | std::ranges::to<std::vector>();

            return run_tests( not_muted, false );
        }

        [[nodiscard]] auto should_test_run( Test const& test ) const -> bool;
        [[nodiscard]] auto matches_config( Test const& test ) const -> bool;
    };

} // namespace CatchKit::Detail

namespace CatchKit {
    using Detail::TestRunner;
}

#endif // CATCH23_RUNNER_H
