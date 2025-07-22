//
// Created by Phil Nash on 22/07/2025.
//

#ifndef CATCH23_COLOUR_H
#define CATCH23_COLOUR_H

namespace CatchKit {

    enum class Colours {
        Reset,
        Red,
        Green,
        Blue
    };

    class TextColour {
    public:
        ~TextColour();

        auto operator=(TextColour&&)  = delete; // non-moveable, non-copyable

        void set(Colours colour) const;
    };

} // namespace CatchKit

#endif //CATCH23_COLOUR_H
