//
// Created by Phil Nash on 23/07/2025.
//

#ifndef CATCH23_SECTIONS_H
#define CATCH23_SECTIONS_H

#include "../src/catch23_internal_execution_nodes.h"

namespace CatchKit::Detail {

    struct SectionInfo {
        ExecutionNode& node;
        bool entered;

        ~SectionInfo() {
            if( entered )
                node.exit();
        }

        explicit operator bool() const noexcept { return entered; }
    };

    auto try_enter_section(ExecutionNodes& nodes, std::string&& name, std::source_location const& location) -> SectionInfo;

} // namespace CatchKit::Detail

#endif //CATCH23_SECTIONS_H
