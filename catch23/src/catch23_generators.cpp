//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23/catch23_generators.h"

#include <map>
#include <cassert>

namespace CatchKit::Detail {

    namespace Charsets {
        std::string const lcase = "abcdefghijklmnopqrstuvwxyz";
        std::string const ucase = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        std::string const all_alpha = lcase + ucase;
        std::string const numbers = "01234567890";
        std::string const alphanumeric = all_alpha + numbers;
        std::string const word_chars = alphanumeric + " ";
        std::string const symbols = "!@£$%^&*()-=_+[]{};'\\\\:\\\"|,./<>?`~";
        std::string const printable_ascii = word_chars + symbols;
    }

    auto generate_value( values_of<std::string> const& generator ) -> std::string {
        auto len = generate_random_number(generator.min_len, generator.max_len);
        std::string str;
        str.resize(len);
        for(size_t i = 0; i < len; ++i)
            str[i] = generator.charset[generate_random_number<size_t>(0, generator.charset.length()-1)];
        return str;
    }

} // namespace CatchKit::Generators::Detail
