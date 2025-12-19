//
// Created by Phil Nash on 15/12/2025.
//

#ifndef CATCH23_CLARA_H
#define CATCH23_CLARA_H

#include <string_view>
#include <vector>
#include <sstream>
#include <expected>
#include <generator>
#include <memory>
#include <cassert>

namespace CatchKit::Clara {

    enum class ParserError {
        MissingArgument,
        ConversionFailure
    };

    template<typename T>
    struct Converter {
        static auto convert_into( std::string_view source, T& target ) -> std::expected<void, ParserError> {
            if constexpr (std::is_constructible_v<std::string_view, T>) {
                target = source;
            }
            else {
                std::stringstream ss;
                ss << source;
                ss >> target;
                if( ss.fail() )
                    return std::unexpected( ParserError::ConversionFailure );
            }
            return {};
        }
    };

    namespace Detail {
        struct FlagPattern {
            enum class Type { Short, Long };
            Type type;
            std::string_view name;

            auto operator <=> ( FlagPattern const& ) const = default;
        };

        enum class FlagPatternError {
            NotAFlag, // Doesn't begin with '-'
            TooManyCharacters, // Short flags (-) may only support single character names
        };

        auto make_flag_pattern( std::string_view flag ) -> std::expected<FlagPattern, FlagPatternError>;
        auto parse_flag_patterns( std::string_view flag_spec ) -> std::vector<FlagPattern>;

        struct Tokens {
            std::generator<std::string_view> token_generator;
            decltype(token_generator.begin()) it;
            decltype(token_generator.end()) end;

            explicit Tokens(std::generator<std::string_view>&& token_generator);

            [[nodiscard]] auto empty() const { return it == end; }
            [[nodiscard]] auto get() const { assert( !empty()); return *it; }
            auto next() { ++it; return !empty(); }
        };

        struct Target {
            Target() = default;
            Target( Target const& ) = delete;
            virtual ~Target() = default;

            virtual auto set_bool( bool value ) -> std::expected<void, ParserError> = 0;
            virtual auto set( std::string_view source ) -> std::expected<void, ParserError> = 0;
        };

        template<typename T>
        struct BoundTarget : Target {
            T& target;
            explicit BoundTarget( T& target ) : target( target ) {}

            auto set_bool( bool value ) -> std::expected<void, ParserError> override {
                if constexpr( std::is_convertible_v<bool, T> ) {
                    target = value;
                    return {};
                }
                else {
                    return std::unexpected( ParserError::ConversionFailure );
                }
            }
            auto set( std::string_view source ) -> std::expected<void, ParserError> override {
                return Converter<T>::convert_into( source, target );
            }
        };
        template<typename F>
        struct BoundInvocable : Target {
            F invocable;
            explicit BoundInvocable( F target ) : invocable( std::move(target) ) {}

            auto set_bool( bool ) -> std::expected<void, ParserError> override {
                // !TBD: Can this be implemented? Does it need to be?
                // Should we accept lambdas that take something other than string_view?
                assert(false);
                return std::unexpected( ParserError::ConversionFailure );
            }
            auto set( std::string_view source ) -> std::expected<void, ParserError> override {
                return invocable( source );
            }
        };

        enum class ParserType { Flag, Opt, Arg };

        struct ParseInfo {
            ParserType type;
            std::shared_ptr<Target> bound_target;
            std::vector<FlagPattern> flags; // For Flags or Options
            std::string_view name; // For Args
            std::string_view desc;

            int min_required = 0;
            int max_allowed = 1;
        };
        struct SingleParser {
            ParseInfo info;
            int consumed = 0;

            [[nodiscard]] auto match_flag( std::string_view token ) -> bool;
            [[nodiscard]] auto try_parse_tokens( Tokens& tokens ) -> std::expected<bool, ParserError>;
            [[nodiscard]] auto is_consumed() const -> bool { return consumed >= info.max_allowed; }
            [[nodiscard]] auto verify() const -> bool { return consumed >= info.min_required && consumed <= info.max_allowed; }
        };

        class Parser {
            std::vector<SingleParser> parsers;
        public:
            auto parse( int argc, char** argv ) -> std::expected<void, ParserError>;
            auto parse( std::vector<std::string_view> const& args ) -> std::expected<void, ParserError>;

            template<typename Self, typename Other>
            auto operator | (this Self&& self, Other&& other) -> Parser {
                Parser composite( std::forward<Self>( self ) );
                composite.parsers.emplace_back( SingleParser(other) );
                return composite;
            }

        private:
            [[nodiscard]] auto try_parse_tokens( Tokens& tokens ) -> std::expected<bool, ParserError>;
            [[nodiscard]] auto verify() const -> bool;
        };


        template<typename T>
        [[nodiscard]] auto bind_target( T& target ) {
            return std::make_shared<BoundTarget<T>>( target );
        }
        template<std::invocable<std::string_view> F>
        [[nodiscard]] auto bind_target( F&& target_invocable ) {
            return std::make_shared<BoundInvocable<F>>( std::forward<F>( target_invocable ) );
        }

        struct ParserBase : ParseInfo {
            ParserBase( ParseInfo&& single_parser )
            : ParseInfo( std::move(single_parser) )
            {}

            template<typename Self>
            auto&& required(this Self&& self) {
                if( self.min_required == 0 )
                    self.min_required = 1;
                return std::forward<Self>(self);
            }

            auto parse( std::vector<std::string_view> const& args ) -> std::expected<void, ParserError> {
                return (Parser() | *this).parse( args );
            }

            template<typename Self, typename Other>
            [[nodiscard]] auto operator | (this Self const& self, Other const& other) -> Parser {
                Parser composite;
                return Parser() | self | other;
            }
        };

        [[nodiscard]] auto parse_bool_yes_no(std::string_view str) -> std::expected<bool, ParserError>;

    } // namespace Detail

    struct Flag : Detail::ParserBase {
        template<typename T>
        Flag( std::string_view spec, std::string_view desc, T&& target )
        : ParserBase( ParseInfo{
                .type = Detail::ParserType::Flag,
                .bound_target = Detail::bind_target( std::forward<T>( target ) ),
                .flags = Detail::parse_flag_patterns(spec),
                .name = {},
                .desc = desc } )
        {}
    };

    struct Opt : Detail::ParserBase {
        template<typename T>
        Opt( std::string_view spec, std::string_view desc, T&& target )
        : ParserBase( ParseInfo{
                .type = Detail::ParserType::Opt,
                .bound_target = Detail::bind_target( std::forward<T>( target ) ),
                .flags = Detail::parse_flag_patterns(spec),
                .name = {},
                .desc = desc } )
        {}
    };

    struct Arg : Detail::ParserBase {
        template<typename T>
        Arg( std::string_view name, std::string_view desc, T&& target )
        : ParserBase( ParseInfo{
                .type = Detail::ParserType::Arg,
                .bound_target = Detail::bind_target( std::forward<T>( target ) ),
                .flags = {},
                .name = name,
                .desc = desc } )
        {}
    };

    using Detail::parse_bool_yes_no;
    using Detail::Parser;

} // namespace CatchKit::Clara

#endif // CATCH23_CLARA_H