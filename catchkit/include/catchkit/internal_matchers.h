//
// Created by Phil Nash on 30/07/2025.
//

#ifndef CATCHKIT_INTERNAL_MATCHERS_H
#define CATCHKIT_INTERNAL_MATCHERS_H

#include "expr_ref.h"
#include "exceptions.h"
#include "stringify.h"

#include <type_traits>
#include <format>
#include <vector>
#include <utility>

namespace CatchKit {

    using Detail::MatchResult;

    namespace Detail {

        struct Matcher {
            // This is just a marker interface
        };

        struct CouldBeAnything {
            template <typename T> explicit(false) operator T() const;
        };

        struct AlwaysMatcher {
            auto match(auto&&) const -> MatchResult { return true; }
            auto lazy_match(auto&&) const -> MatchResult  { return true; }
        };

        // !TBD: refactor this all in terms of marker interfaces, or something
        // * currently we don't handle overloads or constrained-template match functions properly
        template<typename M, typename T=CouldBeAnything>
        concept IsEagerMatcher = requires(M m, T something) {
            { m.match(something) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsLazyMatcher = requires(M m) {
            { m.lazy_match([]{}) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsEagerBindableMatcher = requires(M m, CouldBeAnything something, AlwaysMatcher matcher) {
            { m.bound_match(something, matcher) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsLazyBindableMatcher = requires(M m, AlwaysMatcher matcher) {
            { m.lazy_bound_match([]{}, matcher) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsMatcher = IsEagerMatcher<M> || IsLazyMatcher<M>;

        template<typename M>
        concept MatcherHasDescribeMethod = requires(M const m) {
            { m.describe() } -> std::convertible_to<std::string>;
        };

        template<typename ArgT, typename MatcherT>
        auto invoke_matcher( MatcherT& matcher, ArgT&& arg ) -> MatchResult {
            if constexpr( IsLazyMatcher<MatcherT> ) {
                auto address = std::bit_cast<uintptr_t>( &matcher );
                if constexpr( std::invocable<ArgT> )
                // static_assert( std::invocable<ArgT>, "Lazy matchers must be matched against lambdas" );
                    return matcher.lazy_match( arg ).set_address( address );
                else if constexpr( IsEagerMatcher<MatcherT> )
                    return matcher.match( arg ).set_address( address );
                else
                    static_assert( false, "Lazy matchers must be matched against lambdas" );
            }
            else if constexpr( IsEagerMatcher<MatcherT> ) {
                auto address = std::bit_cast<uintptr_t>( &matcher );
                if constexpr( std::invocable<ArgT> )
                    return matcher.match( arg() ).set_address( address );
                else
                    return matcher.match( arg ).set_address( address );
            }
            else {
                static_assert( false, "RHS of match statement is not a matcher" );
            }
        }


        // The result of a && expression
        template<typename M1, typename M2>
        struct AndMatcher {
            M1& matcher1;
            M2& matcher2;

            auto match( auto const& value ) const -> MatchResult {
                auto result1 = invoke_matcher( matcher1, value ).make_child_of(this);
                if( !result1 )
                    return result1; // Short circuit
                return invoke_matcher( matcher2, value )
                    .make_child_of(this) // Create new matcher for this level
                    .add_children_from(result1); // add in the other result

            }
            [[nodiscard]] auto describe() const {
                return std::format("({} && {})", matcher1.describe(), matcher2.describe());
            }
        };

        // The result of a || expression
        template<typename M1, typename M2>
        struct OrMatcher {
            using composite_matcher = void;

            M1& matcher1;
            M2& matcher2;

            auto match( auto const& value ) const -> MatchResult {
                auto result1 = invoke_matcher(matcher1, value).make_child_of(this);
                if( result1 )
                    return result1; // Short circuit
                return invoke_matcher( matcher2,  value )
                    .make_child_of(this) // Create new matcher for this level
                    .add_children_from(result1); // add in the other result
            }
            [[nodiscard]] auto describe() const {
                return std::format("({} || {})", matcher1.describe(), matcher2.describe());
            }
        };

        // The result of a ! expression
        template<typename M>
        struct NotMatcher {
            M& base_matcher;

            auto match( auto const& value ) const -> MatchResult {
                return match_common( value );
            }
            // !TBD: only include lazy_match if the base matcher is lazy
            auto lazy_match( auto const& value ) const -> MatchResult {
                return match_common( value );
            }
            auto match_common( auto const& value ) const -> MatchResult {
                auto result = invoke_matcher(base_matcher, value).make_child_of(this);
                result.result = !result.result;
                return result;

            }
            [[nodiscard]] auto describe() const {
                return std::format("!({})", base_matcher.describe());
            }
        };

        template<typename M2>
        auto operator && (IsMatcher auto&& m1, M2&& m2) {
            static_assert(IsMatcher<M2>, "Operand to && is not a matcher");
            return AndMatcher(m1, m2);
        }

        template<typename M2>
        auto operator || (IsMatcher auto&& m1, M2&& m2) {
            static_assert(IsMatcher<M2>, "Operand to || is not a matcher");
            return OrMatcher( m1, m2 );
        }

        auto operator ! (IsMatcher auto&& m) {
            return NotMatcher(m);
        }

        // Matchers may be monadically bound with the >>= operator.
        // If so, the left operand must implement bound_match() or lazy_bound_match()
        template<typename M1, typename M2>
        struct BoundMatchers {
            M1 matcher1;
            M2 matcher2;

            template<typename ArgT>
            auto lazy_match( ArgT const& arg ) const -> MatchResult {
                if constexpr ( IsLazyBindableMatcher<M1> ) {
                    static_assert( std::invocable<ArgT>, "Lazy matchers must be matched against lambdas" );
                    return matcher1.lazy_bound_match(arg, matcher2)
                        .set_address( std::bit_cast<uintptr_t>(&matcher1) )
                        .make_child_of(this);
                }
                else
                    return match(arg);
            }

            template<typename ArgT>
            auto match( ArgT const& arg ) const -> MatchResult {
                static_assert( IsEagerBindableMatcher<M1>, "The LHS of >>= must be a bindable matcher" );
                if constexpr( std::invocable<ArgT> )
                    return matcher1.bound_match(arg(), matcher2)
                        .set_address( std::bit_cast<uintptr_t>(&matcher1) )
                        .make_child_of(this);
                else
                    return matcher1.bound_match(arg, matcher2)
                        .set_address( std::bit_cast<uintptr_t>(&matcher1) )
                        .make_child_of(this);
            }

            [[nodiscard]] auto describe() const -> std::string {
                return std::format("({} >>= {})", matcher1.describe(), matcher2.describe());
            }
        };

        template<IsMatcher M1, typename M2>
        auto operator >>= ( M1&& m1, M2&& m2 ) {
            static_assert(IsMatcher<M2>, "Operand to >>= is not a matcher");
            return BoundMatchers{std::forward<M1>(m1), std::forward<M2>(m2)};
        }

        template<typename ArgT, typename MatcherT>
        [[maybe_unused]] constexpr auto operator, ( MatchExprRef<ArgT, MatcherT>&& matcher_ref, std::string_view message ) noexcept {
            matcher_ref.message = message;
            return matcher_ref;
        }

        template<typename M>
        concept IsBinaryCompositeMatcher = requires(M const m) {
            { m.matcher1 } -> IsMatcher;
            { m.matcher2 } -> IsMatcher;
        };

        template<typename M>
        concept IsUnaryCompositeMatcher = requires(M const m) {
            { m.base_matcher } -> IsMatcher;
        };

        template<typename M>
        concept IsCompositeMatcher = IsBinaryCompositeMatcher<M> || IsUnaryCompositeMatcher<M>;

        void add_subexpressions( std::vector<SubExpressionInfo>& sub_expressions, MatchResult const& results, uintptr_t matcher_address, std::string const& description );

        template<typename M>
        auto collect_subexpressions(M const& matcher, std::vector<SubExpressionInfo>& sub_expressions, MatchResult const& results) {
            if constexpr( IsBinaryCompositeMatcher<M> ) {
                collect_subexpressions(matcher.matcher1, sub_expressions, results);
                collect_subexpressions(matcher.matcher2, sub_expressions, results);
            }
            else if constexpr( IsUnaryCompositeMatcher<M> ) {
                collect_subexpressions(matcher.base_matcher, sub_expressions, results);
            }
            else {
                add_subexpressions( sub_expressions, results, std::bit_cast<uintptr_t>(&matcher), matcher.describe() );
            }
        }

        template<typename ArgT, typename MatcherT>
        auto MatchExprRef<ArgT, MatcherT>::evaluate() -> MatchResult {
            return invoke_matcher( matcher, arg );
        }

        template<typename ArgT, typename MatcherT>
        auto MatchExprRef<ArgT, MatcherT>::expand( MatchResult const& result ) -> ExpressionInfo {
            std::vector<SubExpressionInfo> sub_expressions;
            if constexpr ( IsCompositeMatcher<MatcherT>) {
                collect_subexpressions(matcher, sub_expressions, result);
            }
            std::string arg_as_string;
            try {
                if constexpr ( !std::invocable<ArgT> )
                    arg_as_string = stringify(arg);
                else if constexpr (!std::is_void_v<decltype(arg())>)
                    arg_as_string = stringify(arg());
            }
            catch(...) {
                arg_as_string = std::format("exception thrown while evaluating matcher: {}", get_current_exception_message() );
            }
            return MatchExpressionInfo{ arg_as_string, matcher.describe(), std::move(sub_expressions) };
        }

    } // namespace Detail

    namespace Matchers {

        using Detail::operator &&;
        using Detail::operator ||;
        using Detail::operator !;
        using Detail::operator >>=;

    } // namespace Matchers

} // namespace CatchKit

#endif // CATCHKIT_INTERNAL_MATCHERS_H
