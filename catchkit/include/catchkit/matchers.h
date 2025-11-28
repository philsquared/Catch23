//
// Created by Phil Nash on 21/07/2025.
//

#ifndef CATCHKIT_MATCHERS_H
#define CATCHKIT_MATCHERS_H

#include "internal_matchers.h"

#include <cmath>
#include <unordered_set>
#include <ranges>
#include <set>

namespace CatchKit {

    template<class T>
    concept IsStringViewable = std::is_convertible_v<T, std::string_view>;

    template<class T>
    concept NotStringViewable = !std::is_convertible_v<T, std::string_view>;

    template<class T>
    concept NonStringRange = NotStringViewable<T> && std::ranges::sized_range<T>;

    template<class T>
    concept NotARange = NotStringViewable<T> && !std::ranges::sized_range<T>;


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
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("equals( {} )", stringify(match_value));
            }
        };

        struct HasSize {
            std::size_t size;

            [[nodiscard]] auto match(auto&& val) const -> MatchResult { // NOSONAR NOLINT(misc-type)
                return std::size(val) == size;
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
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
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return description;
            }
        };
    }

    namespace RangeMatchers {

        struct InOrder {
            static auto equals(auto const& range1, auto const& range2 ) {
                if( std::size(range1) != std::size(range2) )
                    return false;
                for( const auto& [e1, e2] : std::views::zip(range1, range2) )
                    if( e1 != e2 )
                        return false;
                return true;
            }
        };
        struct InAnyOrder {
            static auto equals(auto const& range1, auto const& range2 ) {
                auto set = range2 | std::ranges::to<std::unordered_set>();
                for( const auto& match_element : range1 )
                    if( set.erase(match_element) == 0 )
                        return false;
                return set.empty();
            }
        };

        template<typename T>
        struct ContainsElement {
            T const& element;

            template<typename Range>
            [[nodiscard]] auto match(Range const& range) const -> MatchResult {
                static_assert(std::ranges::sized_range<Range>);
                for( const auto& match_element : range ) {
                    if( element == match_element )
                        return true;
                }
                return false;
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("contains({})", stringify(element));
            }
        };

        template<typename R>
        struct ContainsRange {
            R const& range;

            template<typename Range>
            [[nodiscard]] auto match(Range const& match_range) const -> MatchResult {
                static_assert(std::ranges::sized_range<Range>);
                auto set = range | std::ranges::to<std::unordered_set>();
                for( const auto& match_element : match_range )
                    set.erase(match_element);
                return set.empty();
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("contains({})", stringify(range));
            }
        };

        template<typename OrderPolicy, typename R>
        struct Equals {
            R const& range;

            template<typename Range>
            [[nodiscard]] auto match(Range const& match_range) const -> MatchResult {
                static_assert(std::ranges::sized_range<Range>);
                return OrderPolicy::equals(range, match_range);
            }

            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("equals({})", stringify(range));
            }
        };
    }


    namespace StringMatchers {

        struct CaseSensitive {
            static constexpr std::string name = "CaseSensitive";
            static bool equal(std::string_view str1, std::string_view str2);
            static bool find(std::string_view str, std::string_view substr);
            static bool matches_regex(std::string const& str, std::string const& regex_str);
        };
        struct CaseInsensitive {
            static constexpr std::string name = "CaseInsensitive";
            static bool equal(std::string_view str1, std::string_view str2);
            static bool find(std::string_view str, std::string_view substr);
            static bool matches_regex(std::string const& str, std::string const& regex_str);
        };

        template<typename CasePolicy=CaseSensitive>
        struct StartsWith {
            std::string_view match_str;

            [[nodiscard]] auto match(std::string_view str) const -> MatchResult {
                if( match_str.size() > str.size() )
                    return false;
                return CasePolicy::equal(str.substr(0, match_str.size()), match_str);
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("starts_with<{}>(\"{}\")", CasePolicy::name, match_str);
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
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("ends_with<{}>(\"{}\")", CasePolicy::name, match_str);
            }
        };
        template<typename CasePolicy=CaseSensitive>
        struct Contains {
            std::string_view match_str;
            [[nodiscard]] auto match( IsStringViewable auto const& str ) const -> MatchResult {
                return CasePolicy::find(str, match_str);
            }
            [[nodiscard]] auto match( NonStringRange auto const& range ) const -> MatchResult {
                // Special case where we are matching a string, but when contained within a range
                for( const auto& element : range ) {
                    if( CasePolicy::equal( match_str, element ) )
                        return true;
                }
                return false;
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("contains<{}>(\"{}\")", CasePolicy::name, match_str);
            }
        };

        template<typename CasePolicy=CaseSensitive>
        struct Equals {
            std::string_view match_str;

            [[nodiscard]] auto match(std::string_view str) const -> MatchResult {
                return CasePolicy::equal(str, match_str);
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("equals<{}>(\"{}\")", CasePolicy::name, match_str);
            }
        };


        template<typename CasePolicy=CaseSensitive>
        struct MatchesRegex {
            std::string regex_str;

            [[nodiscard]] auto match(std::string const& str) const -> MatchResult {
                return CasePolicy::matches_regex(str, regex_str);
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("matches_regex<{}>(\"{}\")", CasePolicy::name, regex_str);
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
            [[nodiscard]] auto match(auto const& ex, BoundMatcherT const& bound_matcher ) const -> CompositeMatchResult {
                static_assert(Detail::IsEagerMatcher<BoundMatcherT, std::string>);

                std::string message = Detail::get_exception_message(ex);
                if( what && message != *what )
                    return false;

                return CompositeMatchResult( bound_matcher.match(message) )
                    .set_address_of( bound_matcher )
                    .make_child_of( this );
            }

            [[nodiscard]] auto describe() const -> MatcherDescription {
                if( what )
                    return std::format("has_message(\"{}\")", *what);
                else
                    return std::format("has_message()");
            }
        };

        template<typename E=void>
        struct Throws {
            template<Detail::IsEagerMatcher<std::string> MessageMatcher>
            auto constexpr with_message_that( MessageMatcher const& message_matcher ) {
                using Detail::operator >>=;
                return *this >>= HasMessage() >>= message_matcher;
            }
            auto constexpr with_message( std::string_view message_to_match ) {
                using Detail::operator >>=;
                return *this >>= HasMessage() >>= StringMatchers::Equals{ message_to_match };
            }

            template<typename ArgT>
            [[nodiscard]] constexpr auto lazy_match(ArgT&& f) const -> CompositeMatchResult {
                return lazy_match(std::forward<ArgT>(f), Detail::AlwaysMatcher()).make_child_of(this);
            }

            template<typename BoundMatcherT>
            [[nodiscard]] constexpr auto lazy_match(auto&& f, BoundMatcherT const& bound_matcher) const -> CompositeMatchResult {
                if constexpr( std::is_void_v<E> ) {
                    try {
                        f();
                        return false;
                    }
                    catch(...) {
                        return CompositeMatchResult( bound_matcher.match(std::current_exception()) )
                            .set_address_of( bound_matcher )
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
                        return CompositeMatchResult( bound_matcher.match(ex) )
                            .set_address_of( bound_matcher )
                            .make_child_of( this );
                    }
                    catch(...) {
                        return false;
                    }
                }
                return false;
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                if constexpr(std::is_void_v<E>)
                    return "throws()";
                else
                    return std::format("throws<{}>()", type_to_string<E>());
            }
        };

    } // namespace ExceptionMatchers

    namespace FloatMatchers {
        inline auto margin_compare( double lhs, double rhs, double margin ) -> bool {
            return (lhs + margin >= rhs) && (rhs + margin >= lhs);
        }

        struct IsCloseToRel {
            double target;
            double epsilon;

            [[nodiscard]] auto match( double value ) const -> MatchResult {
                if( epsilon < 0 || epsilon >= 1 )
                    throw std::domain_error( "epsilon must be positive and < 1" );
                auto rel_margin = epsilon * (std::max)(std::fabs(value), std::fabs(target));
                return margin_compare( value, target, std::isinf(rel_margin) ? 0 : rel_margin );
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("is_close_to({})", target);
            }
        };

        template<std::floating_point T>
        struct IsCloseToAbs {
            T target;
            double margin;

            [[nodiscard]] auto match(T value) const -> MatchResult {
                if( margin < 0 )
                    throw std::domain_error( "margin must be positive" );
                return margin_compare( value, target, margin );
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("is_close_to_abs({})", target);
            }
        };

        template<std::floating_point T>
        struct IsWithinUlp {
            T target;
            std::uint64_t ulps;

            static constexpr T infinity = std::numeric_limits<T>::infinity();

            [[nodiscard]] auto match(T value) const -> MatchResult {
                if constexpr( std::same_as<T, float> ) {
                    if( ulps >= (std::numeric_limits<std::uint32_t>::max)())
                        throw std::domain_error( "ulps is too large for a float" );
                }
                return value == target ||
                    ( std::nextafter( target, infinity )*ulps >= value &&
                    std::nextafter( target, -infinity )*ulps <= value );
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("is_within_ulp( {} )", target);
            }
        };

        struct IsNaN {
            [[nodiscard]] auto match(std::floating_point auto value) const -> MatchResult {
                return std::isnan( value );
            }
            [[nodiscard]] auto describe() const -> MatcherDescription {
                return std::format("is_nan()");
            }
        };
    } // namespace FloatMatchers


    namespace Matchers {
        using StringMatchers::CaseSensitive;
        using StringMatchers::CaseInsensitive;
        using RangeMatchers::InOrder;
        using RangeMatchers::InAnyOrder;

        auto equals(NotARange auto& value) { return GenericMatchers::Equals{value}; }

        inline auto has_size(std::size_t size) { return GenericMatchers::HasSize{size}; }

        template<typename CasePolicy=CaseSensitive>
        auto starts_with(std::string_view str) { return StringMatchers::StartsWith<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        auto ends_with(std::string_view str) { return StringMatchers::EndsWith<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        auto contains(IsStringViewable auto const& str) {
            return StringMatchers::Contains<CasePolicy>{ std::string_view(str) };
        }

        auto contains(NotARange auto const& element) {
            return RangeMatchers::ContainsElement{ element };
        }

        auto contains(NotStringViewable auto const& range) {
            return RangeMatchers::ContainsRange{ range };
        }

        template<typename OrderPolicy = InOrder, NonStringRange R>
        auto equals( R const& range ) {
            return RangeMatchers::Equals<OrderPolicy, R>{ range };
        }

        template<typename CasePolicy=CaseSensitive>
        auto equals(std::string_view str) { return StringMatchers::Equals<CasePolicy>{str}; }

        template<typename CasePolicy=CaseSensitive>
        auto matches_regex(std::string str) { return StringMatchers::MatchesRegex<CasePolicy>{ std::move(str) }; }

        template<std::floating_point T>
        auto is_close_to(T target, double margin = 100*std::numeric_limits<T>::epsilon() ) {
            return FloatMatchers::IsCloseToAbs{ target, margin };
        }
        template<std::floating_point T>
        auto is_close_to_rel( T target, double epsilon = 100*std::numeric_limits<T>::epsilon() ) {
            return FloatMatchers::IsCloseToRel{ target, epsilon };
        }
        auto is_within_ulp(std::floating_point auto target, std::uint64_t ulps=1) {
            return FloatMatchers::IsWithinUlp{ target, ulps };
        }
        inline auto is_nan() {
            return FloatMatchers::IsNaN();
        }

        inline auto is_true() { static bool true_value = true; return equals(true_value); }
        inline auto is_false() { static bool false_value = false; return equals(false_value); }


        template<typename E=void>
        using throws = ExceptionMatchers::Throws<E>;

        template<typename PredicateT>
        auto matches_predicate(PredicateT&& pred, std::string description = "predicate") {
            return GenericMatchers::MatchesPredicate<PredicateT>(std::forward<PredicateT>(pred), std::move(description));
        }

    } // namespace Matchers

} // namespace CatchKit

#endif // CATCHKIT_MATCHERS_H
