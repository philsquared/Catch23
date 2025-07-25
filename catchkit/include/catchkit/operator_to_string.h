//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_OPERATOR_TO_STRING_H
#define CATCHKIT_OPERATOR_TO_STRING_H

#include "operators.h"

namespace CatchKit::Detail {

    // String conversions for operator enum
    template<Operators op>
    [[nodiscard]] consteval auto operator_to_string() noexcept
    {
        using enum Operators;
        if constexpr( op == Equals )                    return "==";
        else if constexpr( op == NotEqualTo )           return "!=" ;
        else if constexpr( op == GreaterThan )          return ">";
        else if constexpr( op == LessThan )             return "<";
        else if constexpr( op == GreaterThanOrEqual )   return ">=";
        else if constexpr( op == LessThanOrEqual )      return "<=";
        else {
            // Note that while None is a valid enum value, we should never use it on a path that leads here
            static_assert( false, "Unrecognised operator" );
        }
    }

} // namespace CatchKit::Detail

#endif // CATCHKIT_OPERATOR_TO_STRING_H
