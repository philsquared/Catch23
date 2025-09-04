//
// Created by Phil Nash on 01/09/2025.
//

#ifndef CATCH23_MOCKS_H
#define CATCH23_MOCKS_H

#include "catchkit/checker.h"

#include <type_traits>
#include <variant>
#include <functional>
#include <generator>
#include <memory>
#include <generator>

// Macros for taking a set of types and adding names, just getting names, or just getting the types
#define ARG_NAME_0()
#define ARG_NAME_1(a) a arg1
#define ARG_NAME_2(a, b) a arg1, b arg2
#define ARG_NAME_3(a, b, c) a arg1, b arg2, c arg3
// !TBD more

#define ARG_0()
#define ARG_1() arg1
#define ARG_2() arg1, arg2
#define ARG_3() arg1, arg2, arg3

#define ARG_TYPE_0()
#define ARG_TYPE_1(a) a
#define ARG_TYPE_2(a, b) a, b
#define ARG_TYPE_3(a, b, c) a, b, c

#define FWD_ARG_0()
#define FWD_ARG_1(a) std::forward<a>(arg1)
#define FWD_ARG_2(a, b) std::forward<a>(arg1), std::forward<b>(arg2)
#define FWD_ARG_3(a, b, c) std::forward<a>(arg1), std::forward<b>(arg2), std::forward<c>(arg3)


#define GET_ARG_MACRO(_0,_1,_2,_3,_4,_5,NAME,...) NAME
#define ADD_ARG_NAMES(...) GET_ARG_MACRO(dummy, ##__VA_ARGS__, ARG_NAME_5, ARG_NAME_4, ARG_NAME_3, ARG_NAME_2, ARG_NAME_1, ARG_NAME_0)(__VA_ARGS__)
#define ADD_ARGS(...) GET_ARG_MACRO(dummy, ##__VA_ARGS__, ARG_5, ARG_4, ARG_3, ARG_2, ARG_1, ARG_0)()
#define FORWARD_ARGS(...) GET_ARG_MACRO(dummy, ##__VA_ARGS__, FWD_ARG_5, FWD_ARG_4, FWD_ARG_3, FWD_ARG_2, FWD_ARG_1, FWD_ARG_0)(__VA_ARGS__)

namespace CatchKit::Mocks {

    struct MatchesAnything {
        bool operator == (auto&&) const { return true; }
    };

    extern const MatchesAnything _;

    struct ExactNumberOfTimes {
        unsigned int value;
        explicit ExactNumberOfTimes( int value ) : value( value ) {}
    };

    struct Cardinality {
        unsigned int min = 0;
        unsigned int max = std::numeric_limits<unsigned int>::max();

        Cardinality( unsigned int min, unsigned int max ) : min( min ), max( max ) { assert( min <= max ); }
        explicit(false) Cardinality( ExactNumberOfTimes times ) : min( times.value ), max( times.value ) {}

        auto is_range() const -> bool { return max > min;}

        [[nodiscard]] auto at_least( ExactNumberOfTimes times ) const { return Cardinality( times.value, max ); }
        [[nodiscard]] auto at_most( ExactNumberOfTimes times ) const { return Cardinality( min, times.value ); }
    };

    [[nodiscard]] inline auto at_least( ExactNumberOfTimes times ) { return Cardinality( times.value, 0 ); }
    [[nodiscard]] inline auto at_most( ExactNumberOfTimes times ) { return Cardinality( 0, times.value ); }

    const ExactNumberOfTimes never(0);
    const ExactNumberOfTimes once(1);
    const Cardinality maybe_once = at_most(once);
    const Cardinality any_number_of_times = Cardinality(0, std::numeric_limits<unsigned int>::max() );

    inline ExactNumberOfTimes operator ""_times(unsigned long long int number_of_times) {
        return ExactNumberOfTimes( static_cast<unsigned int>(number_of_times) );
    }

    struct ExpectationsBase;
    struct MockBase;
    struct MethodBase;

    struct ExpectationBuilderBase {};

    template<typename T>
    concept IsExpectation = std::is_base_of_v<ExpectationBuilderBase, std::remove_cvref_t<T>>;

    struct IExpectation {
        bool is_wildcard = false;
        virtual ~IExpectation() = default;

        virtual void set(ExpectationsBase* root) = 0;
        virtual void reset() = 0;
        virtual void find_expectations_for_method( MockBase* obj_addr, MethodBase* method_addr, std::vector<IExpectation*>& matching ) = 0;
        virtual void verify() const = 0;
    };

    template<typename Ret, typename... Args>
    struct Method;

    struct MethodBase {
        std::string name;
        MockBase* mock_base;
        bool is_wildcard;
        std::source_location location;

        auto get_root_expectation() -> ExpectationsBase*;
        void fail(std::string const& message) const;
        auto get_qualified_name() const -> std::string;

        template<typename T> T try_make_default_return_value() const {
            if constexpr( !std::is_void_v<std::decay_t<T>> ) {
                if constexpr( std::is_default_constructible_v<std::decay_t<T>> )
                    return T();
                else {
                    FAIL(name + ": unable to make default return type");
                    std::unreachable();
                }
            }
        }
    };

    struct ExpectationImplBase : IExpectation {
        MethodBase* method;
        std::source_location location;
        Cardinality cardinality = any_number_of_times;
        std::size_t current_handler = 0;
        std::size_t invocations = 0;

        explicit ExpectationImplBase( MethodBase* method, std::source_location location );

        void fail(std::string const& message) const;
        void verify_within_max() const;

        void set(ExpectationsBase* root) final;
        void reset() final;
        void find_expectations_for_method( MockBase* obj_addr, MethodBase* method_addr, std::vector<IExpectation*>& matching ) final;
        void verify() const final;
    };

    template<typename Ret, typename... Args>
    struct ExpectationImpl : ExpectationImplBase {
        using ThisMethod = Method<Ret, Args...>;
        using MethodSig = std::function<Ret(Args...)>;
        using MatcherSig = std::function<bool(Args...)>;
        std::vector<MethodSig> handlers;
        std::vector<MatcherSig> matchers;

        explicit ExpectationImpl( ThisMethod* method, std::source_location location ) : ExpectationImplBase( method, location ) {}

        auto matches(Args&&... args) -> bool {
            for( auto&& matcher : matchers ) {
                if( !matcher(args...) )
                    return false;
            }
            return true;
        }

        auto invoke(Args&&... args) -> Ret {
            REQUIRE( is_wildcard == false );
            invocations++;
            verify_within_max();
            if( handlers.empty() )
                return method->try_make_default_return_value<Ret>();

            auto& handler = handlers[current_handler++];
            if( current_handler == handlers.size() )
                current_handler = 0;
            assert(handler);
            return handler( args... );
        }
    };


    template<typename Ret, typename... Args>
    struct Expectation : ExpectationBuilderBase {
        using Impl = ExpectationImpl<Ret, Args...>;

        std::unique_ptr<Impl> impl;

        explicit Expectation( Method<Ret, Args...>* method, std::source_location location = std::source_location::current() )
        : impl(std::make_unique<Impl>(method, location)) {}

        auto called( Cardinality cardinality, std::source_location location = std::source_location::current() ) && -> Expectation&& {
            impl->location = location;
            impl->cardinality = cardinality;
            return std::move(*this);
        }

        auto while_matching(auto&&...) && -> Expectation&& {
            // !TBD
            return std::move(*this);
        }
        auto returns( auto const& return_value, std::source_location location = std::source_location::current() ) && -> Expectation&& {
            impl->location = location;
            impl->handlers.emplace_back( [return_value]{ return return_value; } );
            return std::move(*this);
        }

        template<typename Lambda>
        auto operator()(Lambda&& handler, std::source_location location = std::source_location::current() ) && -> Expectation&& {
            impl->location = location;
            impl->handlers.emplace_back( std::forward<Lambda>(handler) );
            assert(impl->handlers.back());
            return std::move(*this);
        }
    };

    struct ExpectationsBase : IExpectation {
        bool is_set = false;
    };

    enum class sequenced {
        in_any_order,
        in_order,
        consecutively,
        independently
    };

    template<sequenced Sequenced>
    struct ExpectationsImpl : ExpectationsBase {
        std::vector<std::unique_ptr<IExpectation>> expectations;

        void set( ExpectationsBase* root ) final {
            REQUIRE( expectations.size() > 0 );
            for( auto const& exp : expectations ) {
                exp->set( root );
            }
        }
        void reset() final {
            for( auto const& exp : expectations )
                exp->reset();
        }

        void find_expectations_for_method( MockBase* obj_addr, MethodBase* method_addr, std::vector<IExpectation*>& matching ) final {
            if constexpr ( Sequenced == sequenced::independently ) {
                for(auto const& exp : std::ranges::reverse_view(expectations) ) {
                    exp->find_expectations_for_method( obj_addr, method_addr, matching );
                }
            }
            else if constexpr ( Sequenced == sequenced::in_order ) {
                if( !expectations.empty() )
                    expectations.front()->find_expectations_for_method( obj_addr, method_addr, matching );
            }
            else {
                // !TBD
                FAIL();
            }
        }
        void verify() const final {
            for( auto&& exp : expectations )
                 exp->verify();
        }
    };

    template<sequenced Sequenced>
    struct Expectations : ExpectationBuilderBase {
        std::unique_ptr<ExpectationsImpl<Sequenced>> impl = std::make_unique<ExpectationsImpl<Sequenced>>();

        Expectations(Expectations&&) noexcept = default;

        template<IsExpectation ...ExpectationTypes>
        explicit Expectations( ExpectationTypes&&... expectation_specs ) {
            (impl->expectations.emplace_back( std::forward_like<ExpectationTypes>(expectation_specs.impl) ), ...);
        }

        ~Expectations() noexcept(false) {
            if( impl && impl->is_set ) {
                impl->is_set = false;
                impl->reset();
                if( std::uncaught_exceptions() == 0 )
                    impl->verify();
            }
        }

        auto set() -> Expectations&& {
            impl->is_set = true;
            impl->set( impl.get() );
            return std::move(*this);
        }
    };

    template<typename T, int C>
    struct MethodImpl {};

    template<typename T, int C>
    struct MethodInfo {};

    template<typename T, int C>
    struct MockMethods :
        MethodInfo<T, C>,
            std::conditional_t<requires { requires MethodInfo<T, C+1>::exists; },
                MockMethods<T, C+1>, std::monostate> {};

    template<typename T>
    struct MethodStartCounter;

    template<typename T>
    struct MockFor : MockMethods<T, 1> {};


    template<typename Ret, typename... Args>
    struct Method : MethodBase {
        Method(std::string const& name, MockBase* mock_base, bool is_wildcard = false, std::source_location location = std::source_location::current() )
        :   MethodBase(name, mock_base, is_wildcard, location)
        {}

        template<typename... MatcherArgs>
        auto operator()(MatcherArgs&&... matcher_args) {
            Expectation<Ret, Args...> exp(this, location);
            static constexpr std::size_t number_of_matchers = sizeof...(MatcherArgs);
            static_assert(number_of_matchers <= sizeof...(Args), "Too many matcher arguments provided");
            auto matcher =
                [matchers_tuple=std::make_tuple(std::forward<MatcherArgs>(matcher_args)...)]
                <typename... LocalArgs>(LocalArgs&&... args) {
                    auto args_tuple = std::make_tuple(std::forward<LocalArgs>(args)...);
                    return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                        return (... && (std::get<Is>(matchers_tuple) == std::get<Is>(args_tuple)));
                    }
                (std::make_index_sequence<number_of_matchers>{});
            };

            exp.impl->matchers.emplace_back( std::move(matcher) );
            return exp;
        }

        template<typename ... InvocationArgs>
        auto invoke(InvocationArgs&&... args) -> Ret {
            if( auto root = get_root_expectation() ) {
                std::vector<IExpectation*> expectations;
                root->find_expectations_for_method(mock_base, this, expectations);
                for( auto* exp_base : expectations ) {
                    if( exp_base->is_wildcard ) {
                        auto* wildcard = static_cast<ExpectationImpl<void>*>(exp_base);
                        wildcard->invocations++;
                        wildcard->verify_within_max();
                        return try_make_default_return_value<Ret>();
                    }
                    auto* exp = static_cast<ExpectationImpl<Ret, Args...>*>(exp_base);
                    if( exp->matches(std::forward<InvocationArgs>(args)...) )
                        return exp->invoke(std::forward<InvocationArgs>(args)...);
                }
                fail(get_qualified_name() + " called, but no expectations found, or all exhausted");
            }
            else {
                fail(get_qualified_name() + " called, but no expectations set. Did you forget to call set()?");
            }
            std::unreachable(); // !TBD make fail noreturn?
       }
    };


    template<sequenced Sequenced=sequenced::independently, IsExpectation ...ExpectationTypes>
    auto expect(ExpectationTypes&&... expectations) {
        return Expectations<Sequenced>( std::forward<ExpectationTypes>(expectations)... );
    }

    struct MockBase {
        static constexpr bool exists = true;
        std::string name;

        Method<void> _;
        ExpectationsBase* expectation = nullptr;


        explicit MockBase(std::string_view interface_name)
        :   name(interface_name),
            _("method wildcard", this, true)
        {}

        virtual ~MockBase() {
            // By the time we get to here any expectation should have been cleaned up
            assert(expectation == nullptr);
        }
    };

} // namespace CatchKit::Mocks

// This macro must be used at global scope
#define MOCK(InterfaceType) \
    namespace for_##InterfaceType { \
        using BaseType = InterfaceType; \
        template<int C> struct MethodImpl {}; \
        template<int C> struct MethodInfo {}; \
        struct NamedMockBase : CatchKit::Mocks::MockBase { NamedMockBase() : MockBase( #InterfaceType ) {} }; \
    } \
    namespace CatchKit::Mocks { \
        template<> struct MethodStartCounter<InterfaceType> { static constexpr int value = __COUNTER__; }; \
        template<int C> struct MethodImpl<InterfaceType, C> : for_##InterfaceType::MethodImpl<C> {}; \
        template<int C> struct MethodInfo<InterfaceType, C> : for_##InterfaceType::MethodInfo<C> {}; \
    } \
    namespace for_##InterfaceType { \
        constexpr int start_counter = CatchKit::Mocks::MethodStartCounter<InterfaceType>::value; \
    } \
    namespace for_##InterfaceType

#define MOCK_METHOD_IMPL(counter, qualifiers, return_type, name, ...) \
    template<> struct MethodImpl<counter-start_counter> : virtual BaseType, virtual NamedMockBase { \
        return_type name ( ADD_ARG_NAMES(__VA_ARGS__) ) qualifiers final; \
    }; \
    template<> struct MethodInfo<counter-start_counter> : MethodImpl<counter-start_counter> { \
        CatchKit::Mocks::Method<return_type __VA_OPT__(,) __VA_ARGS__> name; \
        MethodInfo() : name( #name, this ) {} \
    }; \
    return_type MethodImpl<counter-start_counter>::name ( ADD_ARG_NAMES(__VA_ARGS__) ) qualifiers { \
        return dynamic_cast<MethodInfo<counter-start_counter>*>(const_cast<MethodImpl<counter-start_counter>*>(this))->name.invoke(FORWARD_ARGS(__VA_ARGS__)); \
    }

#define MOCK_METHOD(qualifiers, return_type, name, ...) MOCK_METHOD_IMPL(__COUNTER__, qualifiers, return_type, name, __VA_ARGS__)

#endif // CATCH23_MOCKS_H