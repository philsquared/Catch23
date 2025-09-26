//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/generators.h"

#include <cassert>

namespace CatchKit::Detail {

    namespace Charsets {
        std::string const lcase = "abcdefghijklmnopqrstuvwxyz"; // NOLINT (misc-typo)
        std::string const ucase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; // NOLINT (misc-typo)
        std::string const all_alpha = lcase + ucase;
        std::string const numbers = "01234567890";
        std::string const alphanumeric = all_alpha + numbers;
        std::string const word_chars = alphanumeric + " ";
        std::string const symbols = R"(!@£$%^&*()-=_+[]{};'\\:\"|,./<>?`~)";
        std::string const printable_ascii = word_chars + symbols;
    }


    auto values_of<std::string>::generate( RandomNumberGenerator& rng ) const -> std::string {
        auto len = rng.generate( min_len, max_len );
        std::string str;
        str.resize(len);
        for( std::size_t i = 0; i < len; ++i )
            str[i] = charset[rng.generate<std::size_t>( 0, charset.length()-1) ];
        return str;
    }

} // namespace CatchKit::Generators::Detail
