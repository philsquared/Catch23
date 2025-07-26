//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_COLOUR_H
#define CATCH23_COLOUR_H

namespace CatchKit {

    enum class Colours {
        Reset = 0,
        White = Reset,
        Red,
        Green,
        Blue,
        Cyan,
        Yellow,
        Grey,

        BoldRed,
        BoldGreen,
        BoldGrey,
        BoldWhite,
        BoldYellow,
        BoldNormal
    };

    struct ColourIntent {
        static Colours FileName;
        static Colours Warning;
        static Colours ResultError;
        static Colours ResultSuccess;
        static Colours ResultExpectedFailure;

        static Colours Error;
        static Colours Success;
        static Colours Skip;

        static Colours OriginalExpression;
        static Colours ReconstructedExpression;

        static Colours SecondaryText;
        static Colours Headers;
    };

    class TextColour {
    public:
        TextColour() = default;
        explicit TextColour(Colours initial_colour) { set(initial_colour); };
        ~TextColour();

        auto operator=(TextColour&&)  = delete; // non-moveable, non-copyable

        void set(Colours colour) const;
    };

} // namespace CatchKit

#endif // CATCH23_COLOUR_H
