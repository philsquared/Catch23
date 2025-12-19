//
// Created by Phil Nash on 16/12/2025.
//

#ifndef CATCH23_COMMAND_LINE_H
#define CATCH23_COMMAND_LINE_H

#include "config.h"
#include "clara.h"

#include <expected>

namespace CatchKit {

    auto make_cli_parser(Config& config) -> Clara::Parser;

    auto parse_config(int argc, char** argv) -> std::expected<Config, int>;

} // namespace CatchKit

#endif // CATCH23_COMMAND_LINE_H