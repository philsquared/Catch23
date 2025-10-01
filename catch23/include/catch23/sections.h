//
// Created by Phil Nash on 23/07/2025.
//

#ifndef CATCH23_SECTIONS_H
#define CATCH23_SECTIONS_H

#include "internal_execution_nodes.h"
#include "test_result_handler.h"

namespace CatchKit::Detail {

    struct SectionInfo {
        ExecutionNode& node;
        bool entered;

        ~SectionInfo();

        explicit operator bool() const noexcept { return entered; }
    };

    auto try_enter_section(ExecutionNodes& nodes, std::string_view name, std::source_location const& location = std::source_location::current()) -> SectionInfo;

} // namespace CatchKit::Detail

#endif // CATCH23_SECTIONS_H
