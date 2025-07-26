//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/exceptions.h"

namespace CatchKit::Detail {

    [[nodiscard]] auto get_exception_message( std::exception_ptr const& ex ) -> std::string {
        std::string message;
        try {
            std::rethrow_exception(ex);
        }
        catch(std::exception& e) {
            return e.what();
        }
        catch (std::string& s) {
            return s;
        }
        catch(...) {
            // !TBD: registry for custom exception translations
            return "<unknown exception type>";
        }
    }

} // namespace CatchKit::Detail