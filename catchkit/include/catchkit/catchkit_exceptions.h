//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXCEPTIONS_H
#define CATCHKIT_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace CatchKit::Detail {

    [[nodiscard]] auto get_exception_message( std::exception_ptr const& ex ) -> std::string;

} // namespace CatchKit::Detail

#endif //CATCHKIT_EXCEPTIONS_H
