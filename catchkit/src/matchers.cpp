//
// Created by Phil Nash on 21/07/2025.
//

#include "catchkit/matchers.h"

#include <algorithm>

namespace CatchKit {

    // !TBD: test the others
    static_assert(Detail::IsMatcher<GenericMatchers::Equals<bool>>);
    static_assert(Detail::IsEagerMatcher<ExceptionMatchers::HasMessage>);
    static_assert(Detail::IsMatcher<ExceptionMatchers::HasMessage>);

    namespace StringMatchers {

        bool CaseSensitive::equal(std::string_view str1, std::string_view str2) {
            return str1 == str2;
        }
        bool CaseSensitive::find(std::string_view str, std::string_view substr) {
            return str.contains(substr);
        }
        bool CaseInsensitive::equal(std::string_view str1, std::string_view str2) {
            return std::ranges::equal(str1, str2,
                [](char a, char b) {
                    return std::tolower(a) == std::tolower(b);
                });
        }
        bool CaseInsensitive::find(std::string_view str, std::string_view substr) {
            return !std::ranges::search(str, substr,
                [](char a, char b) {
                    return std::tolower(a) == std::tolower(b);
                }).empty();
        }

    } // namespace StringMatchers

    namespace Detail {
        void add_subexpressions( std::vector<SubExpressionInfo>& sub_expressions, MatchResult const& results, uintptr_t matcher_address, std::string const& description ) {
            if( auto it = std::ranges::find( results.child_results, matcher_address, &SubExpression::matcher_address ); it != results.child_results.end() )
                sub_expressions.emplace_back(description, it->result);
            else if( results.matcher_address == matcher_address )
                // !TBD: Should we ever reach here?
                sub_expressions.emplace_back(description, results.result);
        }
    } // namespace Detail

} // namespace CatchKit