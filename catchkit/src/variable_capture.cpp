//
// Created by Phil Nash on 31/07/2025.
//

#include "catchkit/variable_capture.h"
#include "catchkit/checker.h"

namespace CatchKit::Detail {

    VariableCapture::VariableCapture(std::string_view name, std::string_view type, Checker& checker)
    : name(name), type(type), result_handler(checker.result_handler) {
        result_handler.add_variable_capture(this);
    }
    VariableCapture::~VariableCapture() {
        result_handler.remove_variable_capture(this);
    }

} // namespace CatchKit::Detail
