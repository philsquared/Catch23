//
// Created by Phil Nash on 22/07/2025.
//

#include "catch23_generators.h"

#include <map>
#include <cassert>

namespace CatchKit::Generators::Detail {

    bool Generator::move_next() {
        assert(current_index < size);
        return ++current_index <= size;
    }

    struct SourceLocationComparator {
        bool operator()(const std::source_location& lhs, const std::source_location& rhs) const {
            if (lhs.file_name() != rhs.file_name())
                return lhs.file_name() < rhs.file_name();
            else
                return lhs.line() < rhs.line();
        }
    };
    std::map<std::source_location, std::unique_ptr<Generator>, SourceLocationComparator> generators;

    auto add_generator(std::unique_ptr<Generator> generator, std::source_location loc) -> Generator* {
        auto it = generators.emplace(loc, std::move(generator)).first;
        return it->second.get();
    }

    auto find_generator(std::source_location loc) -> Generator* {
        if( auto it = generators.find(loc); it != generators.end() )
            return it->second.get();
        return nullptr;
    }

} // namespace CatchKit::Generators::Detail
