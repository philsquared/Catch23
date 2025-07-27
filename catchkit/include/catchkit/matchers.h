//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_MATCHERS_H
#define CATCHKIT_MATCHERS_H

#include "expr_ref.h"
#include "exceptions.h"
#include "stringify.h"

#include <type_traits>
#include <format>
#include <cmath>
#include <vector>
#include <utility>


namespace CatchKit {

    using Detail::MatchResult;

    namespace Detail {
        struct CouldBeAnything {
            template <typename T> explicit(false) operator T() const;
        };

        struct AlwaysMatcher {
            auto matches(auto&&) const -> MatchResult { return true; }
            auto matches_lazy(auto&&) const -> MatchResult  { return true; }
        };

        // !TBD: This needs a solution for Matchers with overloads of matches
        template<typename M, typename T=CouldBeAnything>
        concept IsEagerMatcher = requires(M m, T something) {
            { m.matches(something) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsLazyMatcher = requires(M m) {
            { m.matches_lazy([]{}) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsEagerChainableMatcher = requires(M m, CouldBeAnything something, AlwaysMatcher matcher) {
            { m.matches_chained(something, matcher) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsLazyChainableMatcher = requires(M m, AlwaysMatcher matcher) {
            { m.matches_lazy_chained([]{}, matcher) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsMatcher = IsEagerMatcher<M> || IsLazyMatcher<M>;

        template<typename M>
        concept MatcherHasDescribeMethod = requires(M const m) {
            { m.describe() } -> std::convertible_to<std::string>;
        };

        template<typename ArgT, typename MatcherT>
        auto invoke_matcher(MatcherT const& matcher, ArgT&& arg) {
            if constexpr( IsLazyMatcher<MatcherT> ) {
                static_assert( std::invocable<ArgT>, "Lazy matchers must be matched against lambdas" );
                return matcher.matches_lazy(arg);
            }
            else if constexpr( IsEagerMatcher<MatcherT> ) {
                if constexpr( std::invocable<ArgT> )
                    return matcher.matches(arg());
                else
                    return matcher.matches(arg);
            }
            else {
                static_assert(false, "RHS of match statement is not a matcher");
            }
        }

        template<typename M1, typename M2>
        struct AndMatcher {
            M1& matcher1;
            M2& matcher2;

            MatchResult matches( auto const& value ) const {
                if( auto result = matcher1.matches(value); !result )
                    return result;
                return matcher2.matches(value);
            }
            constexpr auto describe() const {
                return std::format("({} && {})", matcher1.describe(), matcher2.describe());
            }
        };

        template<typename M1, typename M2>
        struct OrMatcher {
            M1& matcher1;
            M2& matcher2;

            MatchResult matches( auto const& value ) const {
                // !TBD: combine results?
                if( auto result1 = matcher1.matches(value); result1 )
                    return result1;
                return matcher2.matches(value);
            }
            auto describe() const {
                return std::format("({} || {})", matcher1.describe(), matcher2.describe());
            }
        };

        template<typename M>
        struct NotMatcher {
            M& base_matcher;

            MatchResult matches( auto const& value ) const {
                return !base_matcher.matches(value);
            }
            MatchResult matches_lazy( auto const& value ) const {
                return !invoke_matcher(base_matcher, value);
                // return !base_matcher.matches(value);
            }
            auto describe() const {
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

        // Matchers may be chained (a sort of Monadic bind) with the >>= operator.
        // If chained the left operand must implement matches_chained or matches_lazy_chained
        template<typename M1, typename M2>
        struct ChainedMatchers {
            M1 matcher1;
            M2 matcher2;

            template<typename ArgT>
            auto matches_lazy( ArgT const& arg ) const -> MatchResult {
                // MatchExprRef::evaluate will always detect us as a lazy matcher, so we need to repeat the logic
                // of seeing if we're actually forwarding on to a lazy or eager matcher, with a lambda or not
                if constexpr ( IsLazyChainableMatcher<M1> ) {
                    static_assert( std::invocable<ArgT>, "Lazy matchers must be matched against lambdas" );
                    return matcher1.matches_lazy_chained(arg, matcher2);
                }
                else
                    return matches(arg);
            }

            template<typename ArgT>
            auto matches(ArgT const& arg) const -> MatchResult {
                static_assert( IsEagerChainableMatcher<M1>, "The LHS of >>= must be a chainable matcher" );
                if constexpr( std::invocable<ArgT> )
                    return matcher1.matches_chained(arg(), matcher2);
                else
                    return matcher1.matches_chained(arg, matcher2);
            }

            auto describe() const -> std::string {
                return std::format("({} >>= {})", matcher1.describe(), matcher2.describe());
            }
        };

        template<IsMatcher M1, typename M2>
        auto operator >>= (M1&& m1, M2&& m2) {
            static_assert(IsMatcher<M2>, "Operand to >>= is not a matcher");
            return ChainedMatchers{std::forward<M1>(m1), std::forward<M2>(m2)};
        }


        template<typename ArgT, IsMatcher MatcherT>
        [[maybe_unused]] constexpr auto operator, ( UnaryExprRef<ArgT>&& arg, MatcherT const& matcher ) noexcept {
            static_assert(MatcherHasDescribeMethod<MatcherT>, "Matcher is missing describe method");
            return MatchExprRef{ arg.value, matcher, std::exchange(arg.asserter, nullptr) };
        }

        template<typename ArgT, typename MatcherT>
        [[maybe_unused]] constexpr auto operator, ( MatchExprRef<ArgT, MatcherT>&& matcher_ref, std::string_view message ) noexcept {
            matcher_ref.message = message;
            return matcher_ref;
        }

        template<typename ArgT, typename MatcherT>
        auto MatchExprRef<ArgT, MatcherT>::evaluate() -> ResultType {
            return invoke_matcher(matcher, arg) ? ResultType::Pass : ResultType::MatchFailed;
        }

        template<typename ArgT, typename MatcherT>
        auto MatchExprRef<ArgT, MatcherT>::expand(ResultType result) -> ExpressionInfo {
            std::string expanded = matcher.describe() + ((result == ResultType::Pass) ? " matched" : " failed to match");
            return ExpressionInfo{ expanded, {}, Operators::None, {} };

        }

    } // namespace Detail

    namespace GenericMatchers {
        template<typename T>
        struct Equals {
            T& match_value;

            [[nodiscard]] constexpr auto matches(std::remove_const_t<T>& value) const -> MatchResult {
                return value == match_value;
            }
            [[nodiscard]] constexpr auto matches(T const& value) const -> MatchResult {
                return value == match_value;
            }
            constexpr auto describe() const {
                return std::format("equals( {} )", stringify(match_value));
            }
        };
    }
    namespace StringMatchers {

        struct CaseSensitive {
            static bool equal(std::string_view str1, std::string_view str2);
            static bool find(std::string_view str, std::string_view substr);
        };
        struct CaseInsensitive {
            static bool equal(std::string_view str1, std::string_view str2);
            static bool find(std::string_view str, std::string_view substr);
        };

        template<typename CasePolicy=CaseSensitive>
        struct StartsWith {
            std::string_view match_str;

            [[nodiscard]] constexpr auto matches(std::string_view str) const -> MatchResult {
                if( match_str.size() > str.size() )
                    return false;
                return CasePolicy::equal(str.substr(0, match_str.size()), match_str);
            }
            constexpr auto describe() const {
                return std::format("starts_with(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct EndsWith {
            std::string_view match_str;

            [[nodiscard]] constexpr auto matches(std::string_view str) const -> MatchResult {
                if( match_str.size() > str.size() )
                    return false;
                return CasePolicy::equal(str.substr(str.size()-match_str.size()), match_str);
            }
            constexpr auto describe() const {
                return std::format("ends_with(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct Contains {
            std::string_view match_str;
            [[nodiscard]] constexpr auto matches(std::string_view str) const -> MatchResult {
                return CasePolicy::find(str, match_str);
            }
            constexpr auto describe() const {
                return std::format("contains(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct Equals {
            std::string_view match_str;

            [[nodiscard]] constexpr auto matches(std::string_view str) const -> MatchResult {
                return CasePolicy::equal(str, match_str);
            }
            constexpr auto describe() const {
                return std::format("equals(\"{}\")", match_str);
            }
        };
    } // namespace StringMatchers

    namespace ExceptionMatchers {

        struct HasMessage {
            std::optional<std::string> what;

            auto matches(auto const& ex) const -> MatchResult {
                return what ? Detail::get_exception_message(ex) == *what : true;
            }
            template<typename ChainedMatcherT>
            auto matches_chained(auto const& ex, ChainedMatcherT const& chained_matcher ) const -> MatchResult {
                static_assert(Detail::IsEagerMatcher<ChainedMatcherT>);

                std::string message = Detail::get_exception_message(ex);
                if( what && message != *what )
                    return false;

                return chained_matcher.matches( message );
            }

            [[nodiscard]] auto describe() const -> std::string {
                if( what )
                    return std::format("has_message(\"{}\")", *what);
                else
                    return std::format("has_message()");
            }
        };

        template<typename E=void>
        struct Throws {
            template<Detail::IsEagerMatcher MessageMatcher>
            auto constexpr with_message_that(MessageMatcher const& message_matcher ) {
                using Detail::operator >>=;
                return *this >>= HasMessage() >>= message_matcher;
            }
            auto constexpr with_message(std::string_view message_to_match) {
                using Detail::operator >>=;
                return *this >>= HasMessage() >>= StringMatchers::Equals(message_to_match);
            }

            template<typename ArgT>
            [[nodiscard]] constexpr auto matches_lazy(ArgT&& f) const -> MatchResult {
                return matches_lazy_chained(std::forward<ArgT>(f), Detail::AlwaysMatcher());
            }

            template<typename ChainedMatcherT>
            [[nodiscard]] constexpr auto matches_lazy_chained(auto&& f, ChainedMatcherT const& chained_matcher) const -> MatchResult {
                if constexpr( std::is_void_v<E> ) {
                    try {
                        f();
                        return false;
                    }
                    catch(...) {
                        return chained_matcher.matches(std::current_exception());
                    }
                }
                else {
                    static_assert(Detail::IsEagerMatcher<ChainedMatcherT, E>,
                        "The chained matcher must accept the type (or a super class of) that was detected as thrown");
                    try {
                        f();
                        return false;
                    }
                    catch(E& ex) {
                        return chained_matcher.matches(ex);
                    }
                    catch(...) {
                        return false;
                    }
                }
                return false;
            }
            [[nodiscard]] auto describe() const -> std::string{
                if constexpr(std::is_void_v<E>)
                    return "throws()";
                else
                    return std::format("throws<{}>()", Detail::parse_templated_name("E"));
            }
        };

    } // namespace ExceptionMatchers

    namespace FloatMatchers {
        struct IsCloseTo {
            double target;

            [[nodiscard]] constexpr auto matches(double value) const -> MatchResult {
                // !TBD: use better approach
                return std::fabs(value-target) < 10*std::numeric_limits<double>::epsilon();
            }
            constexpr auto describe() const {
                return std::format("is_close_to({})", target);
            }
        };
    } // namespace FloatMatchers

    namespace VectorMatchers {
        template<typename T, typename AllocatorT>
        struct Equals {
            std::vector<T, AllocatorT> const& match_vec;

            [[nodiscard]] constexpr auto matches(auto const& vec) const -> MatchResult {
                if (match_vec.size() != vec.size())
                    return false;
                return std::equal(match_vec.begin(), match_vec.end(), vec.begin());
            }
            constexpr auto describe() const {
                return std::format("equals(vec)"); // !TBD
            }
        };
    }

    namespace Matchers {
        using StringMatchers::CaseSensitive;
        using StringMatchers::CaseInsensitive;

        template<class T>
        concept NotStringViewable = !std::is_convertible_v<T, std::string_view>;

        template<NotStringViewable T>
        constexpr auto equals(T& value) { return GenericMatchers::Equals<T>{value}; }

        template<NotStringViewable T>
        constexpr auto equals(T&& value) { return GenericMatchers::Equals<T>{value}; }

        template<typename CasePolicy=CaseSensitive>
        constexpr auto starts_with(std::string_view str) { return StringMatchers::StartsWith<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        constexpr auto ends_with(std::string_view str) { return StringMatchers::EndsWith<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        constexpr auto contains(std::string_view str) { return StringMatchers::Contains<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        constexpr auto equals(std::string_view str) { return StringMatchers::Equals<CasePolicy>{str}; }

        constexpr auto is_close_to(double target) { return FloatMatchers::IsCloseTo{target}; }

        constexpr auto is_true() { static bool true_value = true; return equals(true_value); }
        constexpr auto is_false() { static bool false_value = false; return equals(false_value); }

        template<typename T, typename AllocatorT>
        constexpr auto equals(std::vector<T, AllocatorT> const& vec) { return VectorMatchers::Equals<T, AllocatorT>{vec}; }

        template<typename T, typename AllocatorT>
        constexpr auto equals(std::vector<T, AllocatorT>& vec) { return VectorMatchers::Equals<T, AllocatorT>{vec}; }

        template<typename E=void>
        using throws = ExceptionMatchers::Throws<E>;

        using Detail::operator &&;
        using Detail::operator ||;
        using Detail::operator !;
        using Detail::operator >>=;

    } // namespace Matchers

} // namespace CatchKit

#endif // CATCHKIT_MATCHERS_H
