//
// Created by Phil Nash on 30/07/2025.
//

#ifndef CATCHKIT_INTERNAL_MATCHERS_H
#define CATCHKIT_INTERNAL_MATCHERS_H

#include "exceptions.h"
#include "stringify.h"
#include "checker.h"

#include <type_traits>
#include <format>
#include <vector>
#include <utility>
#include <bit>

namespace CatchKit {

    namespace Detail {

        struct SubExpression {
            bool result;
            uintptr_t matcher_address;
        };

        // Holds the result of a match
        struct MatchResult {
            bool result;
            uintptr_t matcher_address;

            explicit(false) MatchResult( bool result, uintptr_t matcher_address = 0 ) : result(result), matcher_address(matcher_address) {}
            explicit operator bool() const { return result; }

            template<typename T, typename Self>
            auto&& set_address_of(this Self&& self, T const& object) {
                self.set_address( std::bit_cast<uintptr_t>( &object ) );
                return std::forward<Self>(self);
            }

        private:
            void set_address( uintptr_t address );
        };

        // A match result for composite matchers (with &&, ||, ! and >>)
        struct CompositeMatchResult : MatchResult { // NOSONAR NOLINT (misc-typo)
            std::vector<SubExpression> child_results;

            using MatchResult::MatchResult;
            explicit(false) CompositeMatchResult( MatchResult const& other ) : MatchResult( other ) {}
            CompositeMatchResult( CompositeMatchResult&& other ) = default;

            auto add_children_from( MatchResult const& ) -> CompositeMatchResult&& { return std::move(*this); }
            auto add_children_from( CompositeMatchResult const& other) -> CompositeMatchResult&&;
            auto make_child_of( uintptr_t address ) -> CompositeMatchResult&&;
            auto make_child_of( auto const& matcher ) -> CompositeMatchResult&& { return make_child_of( std::bit_cast<uintptr_t>( matcher ) ); }
        };

        struct MatcherDescription {
            std::string description;

            template<typename T> requires std::is_convertible_v<T, std::string>
            explicit(false) MatcherDescription( T&& description) // NOSONAR NOLINT (misc-typo)
            : description(std::forward<T>( description )) {}
        };

        inline auto to_result_type( MatchResult const& result ) -> ResultType { return result ? ResultType::Passed : ResultType::Failed; }

        struct AlwaysMatcher {
            static auto match(auto&&) -> MatchResult { return true; }
            static auto lazy_match(auto&&) -> MatchResult  { return true; }
        };

        template<typename M>
        concept IsMatcher = requires(M m) {
            { m.describe() } -> std::same_as<MatcherDescription>;
        };

        template<typename M, typename T>
        concept IsEagerMatcher = requires(M m, T arg) {
            { m.match(arg) } -> std::convertible_to<MatchResult>;
        };

        template<typename M, typename T>
        concept IsLazyMatcher = requires(M m, T(*f)()) {
            { m.lazy_match(f) } -> std::convertible_to<MatchResult>;
        };

        template<typename M, typename T>
        concept IsEagerBindableMatcher = requires(M m, T arg, AlwaysMatcher matcher) {
            { m.match(arg, matcher) } -> std::convertible_to<MatchResult>;
        };

        template<typename M, typename T>
        concept IsLazyBindableMatcher = requires(M m, T(*f)(), AlwaysMatcher matcher) {
            { m.lazy_match(f, matcher) } -> std::convertible_to<MatchResult>;
        };

        template<typename M>
        concept IsCompositeMatcher = requires {
            typename std::remove_cvref_t<M>::ComposedMatcher1;
        };

        template<typename M>
        concept IsBinaryCompositeMatcher = requires {
            typename M::ComposedMatcher1;
            typename M::ComposedMatcher2;
        };

        template<typename M>
        concept IsUnaryCompositeMatcher = IsCompositeMatcher<M> && !IsBinaryCompositeMatcher<M>;

        template<typename MatcherT>
        constexpr void enforce_composite_matchers_are_rvalues() {
            static_assert(!IsCompositeMatcher<MatcherT> || !std::is_lvalue_reference_v<MatcherT>,
                "Composite Matchers (&&, ||, !) cannot be stored in variables. Use them inline");
        }


        template<typename ArgT, typename MatcherT>
        auto invoke_matcher( MatcherT& matcher, ArgT&& arg ) {
            if constexpr( std::invocable<ArgT> ) {
                using ReturnedArgType = decltype(arg());
                if constexpr( IsLazyMatcher<MatcherT, ReturnedArgType> ) {
                    return matcher.lazy_match( arg ).set_address_of( matcher );
                }
                else {
                    static_assert( IsEagerMatcher<MatcherT, ReturnedArgType> );
                    return matcher.match( arg() ).set_address_of( matcher );
                }
            }
            else {
                static_assert( IsEagerMatcher<MatcherT, ArgT> );
                return matcher.match( arg ).set_address_of( matcher );
            }
        }


        // The result of a && expression
        template<typename M1, typename M2>
        struct AndMatcher {
            using ComposedMatcher1 = M1;
            using ComposedMatcher2 = M2;
            M1& matcher1;
            M2& matcher2;

            auto match( auto const& value ) const -> CompositeMatchResult {
                auto result1 = CompositeMatchResult( invoke_matcher( matcher1, value ) ).make_child_of(this);
                if( !result1 )
                    return result1; // Short circuit
                return CompositeMatchResult(invoke_matcher( matcher2, value ))
                    .make_child_of(this) // Create new matcher for this level
                    .add_children_from(result1); // add in the other result

            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("({} && {})", matcher1.describe().description, matcher2.describe().description);
            }
        };

        // The result of a || expression
        template<typename M1, typename M2>
        struct OrMatcher {
            using ComposedMatcher1 = M1;
            using ComposedMatcher2 = M2;
            M1& matcher1;
            M2& matcher2;

            auto match( auto const& value ) const -> CompositeMatchResult {
                auto result1 = CompositeMatchResult( invoke_matcher(matcher1, value) )
                    .make_child_of(this);
                if( result1 )
                    return result1; // Short circuit
                return CompositeMatchResult( invoke_matcher( matcher2,  value ) )
                    .make_child_of(this) // Create new matcher for this level
                    .add_children_from(result1); // add in the other result
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("({} || {})", matcher1.describe().description, matcher2.describe().description);
            }
        };

        // The result of a ! expression
        template<typename M>
        struct NotMatcher {
            using ComposedMatcher1 = M;
            M& base_matcher;

            auto match( auto const& value ) const -> CompositeMatchResult {
                return match_common( value );
            }
            auto lazy_match( auto const& value ) const -> CompositeMatchResult {
                return match_common( value );
            }
            auto match_common( auto const& value ) const -> CompositeMatchResult {
                auto result = CompositeMatchResult( invoke_matcher( base_matcher, value ) )
                    .make_child_of(this);
                result.result = !result.result;
                return result;

            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("!({})", base_matcher.describe().description);
            }
        };

        template<IsMatcher M1, typename M2>
        auto operator && ( M1&& m1, M2&& m2 ) { // NOSONAR NOLINT (misc-typo)
            enforce_composite_matchers_are_rvalues<M1>();
            enforce_composite_matchers_are_rvalues<M2>();
            static_assert(IsMatcher<M2>, "Operand to && is not a matcher");
            return AndMatcher(m1, m2);
        }

        template<IsMatcher M1, typename M2>
        auto operator || ( M1&& m1, M2&& m2 ) { // NOSONAR NOLINT (misc-typo)
            enforce_composite_matchers_are_rvalues<M1>();
            enforce_composite_matchers_are_rvalues<M2>();
            static_assert(IsMatcher<M2>, "Operand to || is not a matcher");
            return OrMatcher( m1, m2 );
        }

        template<IsMatcher MatcherT>
        auto operator ! (MatcherT&& m) { // NOSONAR NOLINT (misc-typo)
            enforce_composite_matchers_are_rvalues<MatcherT>();
            return NotMatcher(m);
        }

        // Matchers may be monadically bound with the >>= operator.
        // If so, the left operand must implement match(arg, bound_matcher)
        template<typename M1, typename M2>
        struct BoundMatchers {
            using ComposedMatcher1 = M1;
            using ComposedMatcher2 = M2;
            M1 matcher1;
            M2 matcher2;

            template<typename ArgT>
            auto lazy_match( ArgT const& arg ) const -> CompositeMatchResult {
                if constexpr ( IsLazyBindableMatcher<M1, ArgT> ) {
                    static_assert( std::invocable<ArgT>, "Lazy matchers must be matched against lambdas" );
                    return matcher1.lazy_match(arg, matcher2)
                        .set_address_of( matcher1 )
                        .make_child_of(this);
                }
                else
                    return match(arg);
            }

            template<typename ArgT>
            auto match( ArgT const& arg ) const -> CompositeMatchResult {
                static_assert( IsEagerBindableMatcher<M1, ArgT>, "The LHS of >>= must be a bindable matcher" );
                if constexpr( std::invocable<ArgT> )
                    return CompositeMatchResult( matcher1.match(arg(), matcher2) )
                        .set_address_of( matcher1 )
                        .make_child_of(this);
                else
                    return CompositeMatchResult( matcher1.match(arg, matcher2) )
                        .set_address_of( matcher1 )
                        .make_child_of(this);
            }

            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("({} >>= {})", matcher1.describe().description, matcher2.describe().description);
            }
        };

        template<IsMatcher M1, typename M2>
        auto operator >>= ( M1&& m1, M2&& m2 ) {
            static_assert(IsMatcher<M2>, "Operand to >>= is not a matcher");
            return BoundMatchers{std::forward<M1>(m1), std::forward<M2>(m2)};
        }

        void add_subexpressions( std::vector<SubExpressionInfo>& sub_expressions, CompositeMatchResult const& results, uintptr_t matcher_address, std::string const& description );

        template<typename M>
        auto collect_subexpressions(M const& matcher, std::vector<SubExpressionInfo>& sub_expressions, CompositeMatchResult const& results) {
            if constexpr( IsBinaryCompositeMatcher<M> ) {
                collect_subexpressions(matcher.matcher1, sub_expressions, results);
                collect_subexpressions(matcher.matcher2, sub_expressions, results);
            }
            else if constexpr( IsUnaryCompositeMatcher<M> ) {
                collect_subexpressions(matcher.base_matcher, sub_expressions, results);
            }
            else {
                add_subexpressions( sub_expressions, results, std::bit_cast<uintptr_t>(&matcher), matcher.describe().description );
            }
        }

        template<typename ArgT, typename MatcherT>
        struct MatchExprRef {
            ArgT& arg;
            MatcherT const& matcher;

            [[nodiscard]] auto evaluate() const {
                return invoke_matcher( matcher, arg );
            }
            [[nodiscard]] auto arg_as_string() const -> std::string {
                try {
                    if constexpr ( !std::invocable<ArgT> )
                        return stringify(arg);
                    else if constexpr (!std::is_void_v<decltype(arg())>)
                        return stringify(arg());
                    return {};
                }
                catch(...) {
                    return std::format("exception thrown while evaluating matcher: {}", get_current_exception_message() );
                }
            }
            [[nodiscard]] auto expand( MatchResult const& ) const -> ExpressionInfo {
                return MatchExpressionInfo{ arg_as_string(), matcher.describe().description, {} };
            }
            [[nodiscard]] auto expand( CompositeMatchResult const& result ) const -> ExpressionInfo {
                std::vector<SubExpressionInfo> sub_expressions;
                if constexpr ( IsCompositeMatcher<MatcherT>) {
                    collect_subexpressions(matcher, sub_expressions, result);
                }
                return MatchExpressionInfo{ arg_as_string(), matcher.describe().description, std::move(sub_expressions) };
            }
        };


        template<typename ArgT, typename MatcherT>
        constexpr void Asserter::assert_that( ArgT&& arg, MatcherT&& matcher ) noexcept { // NOSONAR (we use the ref in its lifetime) NOLINT (misc-typo)
            enforce_composite_matchers_are_rvalues<MatcherT>();
            accept_expr( MatchExprRef{ arg, matcher } );
        }

    } // namespace Detail

    namespace Matchers {

        using Detail::operator &&;
        using Detail::operator ||;
        using Detail::operator !;
        using Detail::operator >>=;

    } // namespace Matchers

    using Detail::MatchResult;
    using Detail::CompositeMatchResult;
    using Detail::MatcherDescription;

} // namespace CatchKit

#endif // CATCHKIT_INTERNAL_MATCHERS_H
