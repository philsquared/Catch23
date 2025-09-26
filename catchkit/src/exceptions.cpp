//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/exceptions.h"

namespace CatchKit::Detail {

    const std::string unknown_exception_message = "<unknown exception type>";

    [[nodiscard]] auto get_exception_message( std::exception const& ex ) -> std::string {
        return ex.what();
    }
    [[nodiscard]] auto get_exception_message( std::string const& str ) -> std::string {
        return str;
    }

    [[nodiscard]] auto get_exception_message( std::exception_ptr const& ex ) -> std::string {
        std::string message;
        try {
            std::rethrow_exception(ex);
        }
        catch(std::exception& e) { // NOSONAR NOLINT (misc-typo)
            return get_exception_message(e);
        }
        catch (std::string& s) {
            return get_exception_message(s);
        }
        catch(...) { // NOSONAR NOLINT (misc-typo)
            // !TBD: registry for custom exception translations
            return unknown_exception_message;
        }
    }
    [[nodiscard]] auto get_current_exception_message() -> std::string {
        return get_exception_message(std::current_exception());
    }

} // namespace CatchKit::Detail