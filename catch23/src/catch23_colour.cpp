//
// Created by Phil Nash on 22/07/2025.
//

#include "../include/catch23/catch23_colour.h"

#include <print>
#include <cassert>
#include <utility>

#if !(defined(__DJGPP__) && defined(__STRICT_ANSI__))
#  include <unistd.h>
#endif

namespace CatchKit {
    namespace Detail {
        bool is_colour_available() {
            static bool colour_available =
#if !(defined(__DJGPP__) && defined(__STRICT_ANSI__))
                isatty(STDOUT_FILENO)
#endif
#if defined(CATCHKIT_PLATFORM_APPLE)
            && !isDebuggerActive()
#endif
            ;
            return colour_available;
        }
    } // namespace Detail

    TextColour::~TextColour() {
        set(Colours::Reset);
    }

    void TextColour::set( Colours colour ) const {
        if ( Detail::is_colour_available() ) {
            std::print("\o{33}[0;{}m", [colour]() {
                switch( colour )
                {
                    using enum Colours;
                    case Reset:     return "0;39";
                    case Red:       return "0;31";
                    case Green:     return "0;32";
                    case Blue:      return "0;34";
                    case Cyan:      return "0;36";
                    case Yellow:    return "0;33";
                    case Grey:      return "1;30";

                    case BoldGrey:      return "0;37";
                    case BoldRed:       return "1;31";
                    case BoldGreen:     return "1;32";
                    case BoldWhite:     return "1;37";
                    case BoldYellow:    return "1;33";
                    case BoldNormal:    return "1;39";
                    default:
                        assert(false);
                        std::unreachable();
                }
            }());
        }
    }

} // namespace CatchKit
