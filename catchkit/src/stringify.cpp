//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/stringify.h"

namespace CatchKit {
    auto pointer_to_string( std::uintptr_t p ) -> std::string {
        return std::format("{:#x}", p);
    }
    auto member_pointer_to_string(void const* p, std::size_t size) -> std::string {
        auto bytes = static_cast<unsigned char const*>(p);

        std::string result = "0x";
        for (size_t i = 0; i < size; ++i)
            result += std::format("{:02x}", bytes[i]);
        return result;
    }
}
