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
    struct TypedVariableCapture : VariableCapture {
        T const& value;

        [[nodiscard]] auto get_value() const -> std::string override { return stringify( value ); }

        TypedVariableCapture(T const& value, std::string_view name, Checker& checker)
        :   VariableCapture( name, get_type_name<T>(), checker ),
            value(value)
        {}
    };

} // namespace CatchKit::Detail

#define CATCHKIT_INTERNAL_DECLARE_VAR_X(suffix, var) CatchKit::Detail::TypedVariableCapture CATCHKIT_INTERNAL_UNIQUE_NAME(var_capture##suffix)(var, #var, check)

#define CATCHKIT_INTERNAL_DECLARE_VARS_1(a) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a)
#define CATCHKIT_INTERNAL_DECLARE_VARS_2(a, b) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a); CATCHKIT_INTERNAL_DECLARE_VAR_X(_2, b)
#define CATCHKIT_INTERNAL_DECLARE_VARS_3(a, b, c) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a); CATCHKIT_INTERNAL_DECLARE_VAR_X(_2, b); CATCHKIT_INTERNAL_DECLARE_VAR_X(_3, c)
#define CATCHKIT_INTERNAL_DECLARE_VARS_4(a, b, c, d) CATCHKIT_INTERNAL_DECLARE_VAR_X(_1, a); CATCHKIT_INTERNAL_DECLARE_VAR_X(_2, b); CATCHKIT_INTERNAL_DECLARE_VAR_X(_3, c); CATCHKIT_INTERNAL_DECLARE_VAR_X(_4, d)

#define DECLARE_VARS_ERROR(...) \
    static_assert(false, \
        "CAPTURE ERROR: Too many variable names provided! " \
        "This macro supports a maximum of 4 variables. " \
        "Consider breaking into multiple CAPTURE calls.")

#define CATCHKIT_INTERNAL_GET_6TH_ARG(arg1, arg2, arg3, arg4, arg5, arg6, ...) arg6
#define CATCHKIT_INTERNAL_DECLARE_VARS_CHOOSER(...) CATCHKIT_INTERNAL_GET_6TH_ARG(__VA_ARGS__, DECLARE_VARS_ERROR, CATCHKIT_INTERNAL_DECLARE_VARS_4, CATCHKIT_INTERNAL_DECLARE_VARS_3, CATCHKIT_INTERNAL_DECLARE_VARS_2, CATCHKIT_INTERNAL_DECLARE_VARS_1)

#define CATCHKIT_INTERNAL_VAR(...) CATCHKIT_INTERNAL_DECLARE_VARS_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#define CAPTURE(...) CATCHKIT_INTERNAL_VAR(__VA_ARGS__)

#endif // CATCHKIT_VARIABLE_CAPTURE_H
