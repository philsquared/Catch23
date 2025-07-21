//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_MATCHERS_H
#define CATCHKIT_MATCHERS_H

#include "catchkit_expr_ref.h"
#include "catchkit_exceptions.h"

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

        // !TBD: This needs a solution for Matchers with overloads of matches
        template<typename M>
        concept IsEagerMatcher = requires(M m, CouldBeAnything something) {
            { m.matches(something) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsLazyMatcher = requires(M m) {
            { m.matches_lazy([]{}) } -> std::same_as<MatchResult>;
        };

        template<typename M>
        concept IsMatcher = IsEagerMatcher<M> || IsLazyMatcher<M>;

        template<typename M>
        concept MatcherHasDescribeMethod = requires(M const m) {
            { m.describe() } -> std::convertible_to<std::string>;
        };


        template<typename M1, typename M2>
        struct AndMatcher {
            M1 const& matcher1;
            M2 const& matcher2;

            MatchResult matches( auto const& value ) const {
                if( auto result = matcher1.matches(value); !result )
                    return result;
                return matcher2.matches(value);
            }
            constexpr auto describe() const {
                return std::format("AndMatcher"); // !TBD
            }
        };

        template<typename M1, typename M2>
        struct OrMatcher {
            M1 const& matcher1;
            M2 const& matcher2;

            MatchResult matches( auto const& value ) const {
                // !TBD: combine results?
                if( auto result1 = matcher1.matches(value); result1 )
                    return result1;
                return matcher2.matches(value);
            }
            auto describe() const {
                return std::format("OrMatcher"); // !TBD
            }
        };

        template<typename M2>
        auto operator && (IsMatcher auto const& m1, M2 const& m2) {
            static_assert(IsMatcher<M2>, "Operand to && is not a matcher");
            return AndMatcher(m1, m2);
        }

        template<typename M2>
        auto operator || (IsMatcher auto const& m1, M2 const& m2) {
            static_assert(IsMatcher<M2>, "Operand to || is not a matcher");
            return OrMatcher(m1, m2);
        }

        // Invokes the lambda and checks if it throws - potentially if it throws a specific type
        // and optionally converts the exception to a string and returns that.
        template<typename E, bool return_message>
        auto check_throws(auto f) -> std::optional<std::string> {
            if constexpr( std::is_void_v<E> ) {
                try {
                    f();
                }
                catch(...) {
                    if constexpr(return_message)
                        return get_exception_message(std::current_exception());
                    else
                        return {""};
                }
            }
            else {
                try {
                    f();
                }
                catch (E& e) {
                    if constexpr(return_message)
                        return get_exception_message(std::current_exception());
                    else
                        return {""};
                }
                catch (...) {
                    return {};
                }
            }
            return {};
        }

        template<typename ArgT, IsMatcher MatcherT>
        [[maybe_unused]] constexpr auto operator, ( UnaryExprRef<ArgT>&& arg, MatcherT const& matcher ) noexcept {
            static_assert(MatcherHasDescribeMethod<MatcherT>, "Matcher is missing describe method");
            return MatchExprRef{ arg.value, matcher, std::exchange(arg.checker, nullptr) };
        }

        template<typename ArgT, typename MatcherT>
        [[maybe_unused]] constexpr auto operator, ( MatchExprRef<ArgT, MatcherT>&& matcher_ref, std::string_view message ) noexcept {
            matcher_ref.message = message;
            return matcher_ref;
        }

        template<typename ArgT, typename MatcherT>
        auto MatchExprRef<ArgT, MatcherT>::evaluate() -> ResultType {
            if constexpr( IsLazyMatcher<MatcherT> ) {
                static_assert( std::invocable<ArgT>, "Lazy matchers must be matched against lambdas" );
                return matcher.matches_lazy(arg) ? ResultType::Pass : ResultType::MatchFailed;
            }
            else {
                if constexpr( std::invocable<ArgT> )
                    return matcher.matches(arg()) ? ResultType::Pass : ResultType::MatchFailed;
                else
                    return matcher.matches(arg) ? ResultType::Pass : ResultType::MatchFailed;
            }
        }

        template<typename ArgT, typename MatcherT>
        auto MatchExprRef<ArgT, MatcherT>::expand(ResultType result) -> ExpressionInfo {
            std::string expanded = matcher.describe() + ((result == ResultType::Pass) ? " matched" : " failed to match");
            return ExpressionInfo{ expanded, {}, Operators::None, {} };

        }

    } // namespace Detail

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
                return CasePolicy::equal(str.substr(match_str.size()-str.size()), match_str);
            }
            constexpr auto describe() const {
                return std::format("ends_with(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct Contains {
            std::string_view match_str;
            [[nodiscard]] constexpr auto matches(std::string_view str) const -> MatchResult {
                return CasePolicy::find(match_str, str);
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
        template<typename E=void>
        struct Throws {
            std::string message; // optional

            template<typename MessageMatcher>
            struct WithMatcher {
                MessageMatcher const& message_matcher;
                [[nodiscard]] constexpr MatchResult matches_lazy(auto f) const {
                    if( auto result = Detail::check_throws<E, true>(f) )
                        return message_matcher.matches(*result);
                    return false;
                }
                constexpr auto describe() const {
                    return std::format("throws().with_message_that( {} )", message_matcher.describe());
                }
            };
            struct WithEqualsMatcher {
                StringMatchers::Equals<> message_matcher;
                [[nodiscard]] constexpr MatchResult matches_lazy(auto f) const {
                    if( auto result = Detail::check_throws<E, true>(f) )
                        return message_matcher.matches(*result);
                    return false;
                }
                constexpr auto describe() const {
                    return std::format("throws().with_message_that( {} )", message_matcher.describe());
                }
            };

            template<Detail::IsEagerMatcher MessageMatcher>
            auto constexpr with_message_that(MessageMatcher const& message_matcher ) {
                return WithMatcher<MessageMatcher> {message_matcher};
            }
            auto constexpr with_message(std::string_view message_to_match) {
                return WithEqualsMatcher {message_to_match};
            }

            [[nodiscard]] constexpr auto matches_lazy(auto f) const -> MatchResult {
                if (message.empty())
                    return Detail::check_throws<E, false>(f).has_value();

                if(auto result = Detail::check_throws<E, true>(f))
                    return *result == message;

                return false;
            }
            constexpr auto describe() const {
                return std::format("throws()"); // !TBD: type and value?
            }
        };

        template<typename E=void>
        struct DoesntThrow {
            [[nodiscard]] constexpr auto matches_lazy(auto f) const -> MatchResult {
                return !Detail::check_throws<E, false>(f).has_value();
            }
            constexpr auto describe() const {
                return std::format("doesnt_throw()"); // !TBD: type and value?
            }
        };

    } // namespace ExceptionMatchers

    namespace FloatMatchers {
        struct IsCloseTo {
            double target;

            [[nodiscard]] constexpr auto matches(double value) const -> MatchResult {
                // !TBD: use better approach
                return std::fabs(value-target) < std::numeric_limits<double>::epsilon();
            }
            constexpr auto describe() const {
                return std::format("is_close_to({})", target);
            }
        };
    } // namespace FloatMatchers

    namespace VectorMatchers {
        template<typename T>
        struct Equals {
            std::vector<T> const& match_vec;

            [[nodiscard]] constexpr auto matches(std::vector<T> const& vec) const -> MatchResult {
                if (match_vec.size() != vec.size())
                    return false;
                return std::equal(match_vec.begin(), match_vec.end(), vec.begin());
            }
            constexpr auto describe() const {
                // return std::format("equals({})", match_vec);
                return std::format("equals(vec)"); // !TBD
            }
        };
    }

    namespace Matchers {
        using StringMatchers::CaseSensitive;
        using StringMatchers::CaseInsensitive;

        template<typename CasePolicy=CaseSensitive>
        constexpr auto starts_with(std::string_view str) { return StringMatchers::StartsWith<CasePolicy>{str}; }
        template<typename CasePolicy=CaseSensitive>
        constexpr auto contains(std::string_view str) { return StringMatchers::Contains<CasePolicy>{str}; }
        template<typename CasePolicy=CaseSensitive>
        constexpr auto equals(std::string_view str) { return StringMatchers::Equals<CasePolicy>{str}; }

        constexpr auto is_close_to(double target) { return FloatMatchers::IsCloseTo{target}; }

        template<typename T>
        constexpr auto equals(std::vector<T> const& vec) { return VectorMatchers::Equals<T>{vec}; }

        template<typename E=void>
        using throws = ExceptionMatchers::Throws<E>;

        template<typename E=void>
        using doesnt_throw = ExceptionMatchers::DoesntThrow<E>;

        using Detail::operator &&;
        using Detail::operator ||;
    }

} // namespace CatchKit

#endif //CATCHKIT_MATCHERS_H
