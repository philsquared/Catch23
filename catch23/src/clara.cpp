//
// Created by Phil Nash on 15/12/2025.
//

#include "catch23/clara.h"

#include <algorithm>

namespace CatchKit::Clara {

    namespace Detail {

        auto parse_bool_yes_no(std::string_view str) -> std::expected<bool, ParserError> {
            if( str == "yes" )
                return true;
            if( str == "no" )
                return false;
            return std::unexpected( ParserError::ConversionFailure );
        }

        auto make_flag_pattern( std::string_view flag ) -> std::expected<FlagPattern, FlagPatternError> {
            if( flag.length() < 2 )
                return std::unexpected(FlagPatternError::NotAFlag);
            if( flag[0] != '-' )
                return std::unexpected(FlagPatternError::NotAFlag);
            if( flag[1] == '-' )
                return FlagPattern{ FlagPattern::Type::Long, flag.substr( 2 ) };
            if( flag.length() > 2 )
                return std::unexpected(FlagPatternError::TooManyCharacters);
            return FlagPattern{ FlagPattern::Type::Short, flag.substr( 1 ) };
        }

        auto parse_flag_patterns( std::string_view flag_spec ) -> std::vector<FlagPattern> {
            std::vector<FlagPattern> flags;
            while(!flag_spec.empty()) {
                auto start = flag_spec.find_first_not_of(' ');
                if(start == std::string_view::npos)
                    break;
                auto this_flag = flag_spec.substr(start);
                if( auto end = this_flag.find_first_of(' '); end != std::string_view::npos ) {
                    flag_spec = this_flag.substr( end );
                    this_flag = this_flag.substr( 0, end );
                }
                else {
                    flag_spec = "";
                }
                auto flag_pattern = Detail::make_flag_pattern( this_flag );
                assert(flag_pattern);
                flags.emplace_back( *flag_pattern );
            }
            return flags;
        }

        Tokens::Tokens(std::generator<std::string_view>&& token_generator)
        :   token_generator( std::move(token_generator) ),
            it( this->token_generator.begin() ),
            end( this->token_generator.end() )
        {}

        auto Parser::parse( int argc, char** argv ) -> std::expected<void, ParserError> {
            std::vector<std::string_view> args;
            args.reserve(argc-1);
            for (int i = 1; i < argc; ++i) {
                args.emplace_back(argv[i]);
            }
            return parse(args);
        }
        auto Parser::parse( std::vector<std::string_view> const& args ) -> std::expected<void, ParserError> {
            Tokens tokens( [&args] -> std::generator<std::string_view> {
                for( auto arg : args ) {
                    if( auto result = make_flag_pattern( arg ); !result && result.error() == FlagPatternError::TooManyCharacters ) {
                        // Only non-space characters allowed
                        assert(arg.find(' ') == std::string_view::npos);
                        for(char flag: arg.substr( 1 ) ) {
                            auto expanded_arg = std::string("-") + flag;
                            co_yield expanded_arg;
                        }
                    }
                    else {
                        co_yield arg;
                    }
                }
            }() );

            while( !tokens.empty() ) {
                auto parse_result = try_parse_tokens( tokens );
                if( !parse_result )
                    return std::unexpected( parse_result.error() );
                if( *parse_result ) {
                    // consume
                }
                else {
                    // store/ error
                }
                tokens.next();
            }

            if( !verify() ) // !TBD: other verification errors?
                return std::unexpected( ParserError::MissingArgument );
            return {};
        }

        auto Parser::try_parse_tokens( Tokens& tokens ) -> std::expected<bool, ParserError> {
            for( auto& parser : parsers ) {
                if( !parser.is_consumed() ) {
                    auto parse_result = parser.try_parse_tokens( tokens );
                    if( !parse_result ) // Parser error
                        return parse_result;
                    if( *parse_result ) // Parsed successfully
                        return true;
                    // No match
                }
            }
            return false;
        }

        [[nodiscard]] auto Parser::verify() const -> bool {
            for( auto const& parser : parsers ) {
                if( !parser.verify() )
                    return false;
            }
            return true;
        }

        auto SingleParser::match_flag( std::string_view token ) -> bool {
            auto maybe_flag = make_flag_pattern( token );
            return maybe_flag && std::ranges::contains(info.flags, *maybe_flag);
        }

        auto SingleParser::try_parse_tokens( Tokens& tokens ) -> std::expected<bool, ParserError> {
            if( info.type != ParserType::Arg && !match_flag( tokens.get() ))
                return false;
            if( info.type == ParserType::Flag ) {
                return info.bound_target->set_bool( true )
                    .transform( [] { return true; } );
            }
            if( info.type == ParserType::Opt ) {
                tokens.next();
                if( tokens.empty() )
                    return std::unexpected( ParserError::MissingArgument );
            }
            return info.bound_target->set( tokens.get() ).transform( [] { return true; } );
        }

    } // namespace Detail

} // namespace CatchKit::Clara