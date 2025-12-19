//
// Created by Phil Nash on 16/12/2025.
//

#include "catch23/command_line.h"
#include "print"

namespace CatchKit {

    auto make_cli_parser( Config& config ) -> Clara::Parser {
        using namespace CatchKit::Clara;
        return
              Flag("-h --help", "help", config.help)
            | Flag("-s --success", "include successful tests in output", config.show_successful_tests)
            // | Flag("-b --break", "break into debugger on failure", config.break_into_debugger)
            // | Opt ("-r --reporter", "reporter to use (defaults to console)", config.reporter)
                // .transform(tolower)
                //.validate(OneOf("console", "xml"))
            | Arg("test name|pattern|tags", "with test or tests to run", config.tests_or_tags);
    }

    // Return either a valid config or a return code
    auto parse_config(int argc, char** argv) -> std::expected<Config, int> {
        Config config;
        auto parser = make_cli_parser(config);
        if( auto result = parser.parse(argc, argv); !result ) {
            std::println("Invalid command line");
            return std::unexpected(1);
        }
        if( config.help ) {
            std::println("Help!" );
            // !TBD: show usage
            return std::unexpected(0);
        }
        // !TBD: any unrecognised args?

        return config;
    }


} // namespace CatchKit
