//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_MATCHERS_H
#define CATCHKIT_MATCHERS_H

#include "internal_matchers.h"

#include <cmath>

namespace CatchKit {

    namespace GenericMatchers {
        template<typename T>
        struct Equals {
            T& match_value;

            [[nodiscard]] constexpr auto match(std::remove_const_t<T>& value) const -> MatchResult {
                return value == match_value;
            }
            [[nodiscard]] constexpr auto match(T const& value) const -> MatchResult {
                return value == match_value;
            }
            [[nodiscard]] auto describe() const {
                return std::format("equals( {} )", stringify(match_value));
            }
        };

        struct HasSize {
            std::size_t size;

            [[nodiscard]] auto match(auto&& val) const -> MatchResult { // NOSONAR NOLINT(misc-type)
                return std::size(val) == size;
            }
            [[nodiscard]] auto describe() const {
                return std::format("has_size(\"{}\")", size);
            }
        };

        template<typename PredicateT>
        struct MatchesPredicate {
            PredicateT pred;
            std::string description;

            explicit MatchesPredicate( PredicateT&& pred, std::string description )
            : pred( std::move(pred) ), description( std::move(description) ) {}

            [[nodiscard]] auto match( auto const& arg ) const -> MatchResult {
                return pred(arg);
            }
            [[nodiscard]] auto describe() const -> std::string {
                return description;
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

            [[nodiscard]] auto match(std::string_view str) const -> MatchResult {
                if( match_str.size() > str.size() )
                    return false;
                return CasePolicy::equal(str.substr(0, match_str.size()), match_str);
            }
            [[nodiscard]] auto describe() const {
                return std::format("starts_with(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct EndsWith {
            std::string_view match_str;

            [[nodiscard]] auto match(std::string_view str) const -> MatchResult {
                if( match_str.size() > str.size() )
                    return false;
                return CasePolicy::equal(str.substr(str.size()-match_str.size()), match_str);
            }
            [[nodiscard]] auto describe() const {
                return std::format("ends_with(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct Contains {
            std::string_view match_str;
            [[nodiscard]] auto match(std::string_view str) const -> MatchResult {
                return CasePolicy::find(str, match_str);
            }
            [[nodiscard]] auto describe() const {
                return std::format("contains(\"{}\")", match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct Equals {
            std::string_view match_str;

            [[nodiscard]] auto match(std::string_view str) const -> MatchResult {
                return CasePolicy::equal(str, match_str);
            }
            [[nodiscard]] auto describe() const {
                return std::format("equals(\"{}\")", match_str);
            }
        };

    } // namespace StringMatchers

    namespace ExceptionMatchers {

        struct HasMessage {
            std::optional<std::string> what;

            [[nodiscard]] auto match(auto const& ex) const -> MatchResult {
                return what ? Detail::get_exception_message(ex) == *what : true;
            }
            template<typename BoundMatcherT>
            [[nodiscard]] auto bound_match(auto const& ex, BoundMatcherT const& bound_matcher ) const -> MatchResult {
                static_assert(Detail::IsEagerMatcher<BoundMatcherT>);

                std::string message = Detail::get_exception_message(ex);
                if( what && message != *what )
                    return false;

                return bound_matcher.match(message)
                    .set_address( std::bit_cast<uintptr_t>(&bound_matcher) )
                    .make_child_of(this);
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
            auto constexpr with_message_that( MessageMatcher const& message_matcher ) {
                using Detail::operator >>=;
                return *this >>= HasMessage() >>= message_matcher;
            }
            auto constexpr with_message( std::string_view message_to_match ) {
                using Detail::operator >>=;
                return *this >>= HasMessage() >>= StringMatchers::Equals{ message_to_match };
            }

            template<typename ArgT>
            [[nodiscard]] constexpr auto lazy_match(ArgT&& f) const -> MatchResult {
                return lazy_bound_match(std::forward<ArgT>(f), Detail::AlwaysMatcher()).make_child_of(this);
            }

            template<typename BoundMatcherT>
            [[nodiscard]] constexpr auto lazy_bound_match(auto&& f, BoundMatcherT const& bound_matcher) const -> MatchResult {
                if constexpr( std::is_void_v<E> ) {
                    try {
                        f();
                        return false;
                    }
                    catch(...) {
                        return bound_matcher.match(std::current_exception())
                            .set_address( std::bit_cast<uintptr_t>(&bound_matcher) )
                            .make_child_of(this);
                    }
                }
                else {
                    static_assert(Detail::IsEagerMatcher<BoundMatcherT, E>,
                        "The bound matcher must accept the type (or a super class of) that was detected as thrown");
                    try {
                        f();
                        return false;
                    }
                    catch(E& ex) {
                        return bound_matcher.match(ex)
                            .set_address( std::bit_cast<uintptr_t>(&bound_matcher) )
                            .make_child_of(this);
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
                    return std::format("throws<{}>()", get_type_name<E>());
            }
        };

    } // namespace ExceptionMatchers

    namespace FloatMatchers {
        struct IsCloseTo {
            double target = 0;
            double epsilon = 100*std::numeric_limits<float>::epsilon();

            [[nodiscard]] auto match(double value) const -> MatchResult {
                return std::fabs(value-target) < epsilon;
            }
            [[nodiscard]] auto describe() const {
                return std::format("is_close_to({})", target);
            }
        };
    } // namespace FloatMatchers

    namespace VectorMatchers {
        template<typename T, typename AllocatorT>
        struct Equals {
            std::vector<T, AllocatorT> const& match_vec;

            template<typename Range>
            [[nodiscard]] auto match(Range const& vec) const -> MatchResult {
                static_assert(std::ranges::sized_range<Range>);
                if( match_vec.size() != std::size(vec) )
                    return false;
                return std::equal( match_vec.begin(), match_vec.end(), std::begin(vec) );
            }
            [[nodiscard]] auto describe() const {
                return std::format("equals({})", stringify(match_vec));
            }
        };
    }

    namespace Matchers {
        using StringMatchers::CaseSensitive;
        using StringMatchers::CaseInsensitive;

        template<class T>
        concept NotStringViewable = !std::is_convertible_v<T, std::string_view>;

        template<NotStringViewable T>
        auto equals(T& value) { return GenericMatchers::Equals<T>{value}; }

        inline auto has_size(std::size_t size) { return GenericMatchers::HasSize{size}; }

        template<typename CasePolicy=CaseSensitive>
        auto starts_with(std::string_view str) { return StringMatchers::StartsWith<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        auto ends_with(std::string_view str) { return StringMatchers::EndsWith<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        auto contains(std::string_view str) { return StringMatchers::Contains<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        auto equals(std::string_view str) { return StringMatchers::Equals<CasePolicy>{str}; }

        inline auto is_close_to(float target) { return FloatMatchers::IsCloseTo{target}; }
        inline auto is_close_to(double target) { return FloatMatchers::IsCloseTo{target}; }
        inline auto is_close_to(float target, float epsilon) { return FloatMatchers::IsCloseTo{target, epsilon}; }
        inline auto is_close_to(double target, double epsilon) { return FloatMatchers::IsCloseTo{target, epsilon}; }

        inline auto is_true() { static bool true_value = true; return equals(true_value); }
        inline auto is_false() { static bool false_value = false; return equals(false_value); }

        template<typename T, typename AllocatorT>
        auto equals(std::vector<T, AllocatorT> const& vec) { return VectorMatchers::Equals<T, AllocatorT>{vec}; }

        template<typename T, typename AllocatorT>
        auto equals(std::vector<T, AllocatorT>& vec) { return VectorMatchers::Equals<T, AllocatorT>{vec}; }

        template<typename E=void>
        using throws = ExceptionMatchers::Throws<E>;

        template<typename PredicateT>
        auto matches_predicate(PredicateT&& pred, std::string description = "predicate") {
            return GenericMatchers::MatchesPredicate<PredicateT>(std::forward<PredicateT>(pred), std::move(description));
        }

    } // namespace Matchers

} // namespace CatchKit

#endif // CATCHKIT_MATCHERS_H
