//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/stringify.h"

namespace CatchKit {
    auto pointer_to_string(void const* p) -> std::string {
        return std::format("{:#x}", std::bit_cast<std::uintptr_t>(p));
    }
}