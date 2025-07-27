//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/exceptions.h"

namespace CatchKit::Detail {

    std::string unknown_exception_message = "<unknown exception type>";

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
        catch(std::exception& e) {
            return get_exception_message(e);
        }
        catch (std::string& s) {
            return get_exception_message(s);
        }
        catch(...) {
            // !TBD: registry for custom exception translations
            return unknown_exception_message;
        }
    }

} // namespace CatchKit::Detail