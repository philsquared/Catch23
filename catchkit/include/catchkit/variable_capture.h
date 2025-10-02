//
// Created by Phil Nash on 31/07/2025.
//

#ifndef CATCHKIT_VARIABLE_CAPTURE_H
#define CATCHKIT_VARIABLE_CAPTURE_H

#include "stringify.h"

#include <string_view>

namespace CatchKit::Detail
{
    struct Checker;
    struct ResultHandler;

    struct VariableCapture {
        std::string_view name;
        std::string_view type;
        ResultHandler& result_handler;

        [[nodiscard]] virtual auto get_value() const -> std::string = 0;

    protected:
        VariableCapture(std::string_view name, std::string_view type, Checker& checker);
        ~VariableCapture(); // not virtual because we never destroy polymorphically
    };

    template<typename T>
    struct TypedVariableCapture final : VariableCapture {
        T const& value;

        [[nodiscard]] auto get_value() const -> std::string override { return stringify( value ); }

        TypedVariableCapture(T const& value, std::string_view name, Checker& checker)
        :   VariableCapture( name, type_to_string<T>(), checker ),
            value(value)
        {}
    };

} // namespace CatchKit::Detail

#endif // CATCHKIT_VARIABLE_CAPTURE_H
