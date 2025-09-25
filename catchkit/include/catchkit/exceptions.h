//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_EXCEPTIONS_H
#define CATCHKIT_EXCEPTIONS_H

#include <string>
#include <exception>

namespace CatchKit::Detail {

    extern const std::string unknown_exception_message;

    [[nodiscard]] auto get_current_exception_message() -> std::string;

    [[nodiscard]] auto get_exception_message( std::exception_ptr const& ex ) -> std::string;
    [[nodiscard]] auto get_exception_message( std::exception const& ex ) -> std::string;
    [[nodiscard]] auto get_exception_message( std::string const& str ) -> std::string;

    [[nodiscard]] inline auto get_exception_message(...) -> std::string { // NOSONAR
        return unknown_exception_message;
    }

} // namespace CatchKit::Detail

#endif // CATCHKIT_EXCEPTIONS_H
