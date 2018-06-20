#ifndef WINDOW_STYLE_HPP
#define WINDOW_STYLE_HPP

#include "glk.hpp"

#define STYLE(n) n = style_##n

namespace Glk {
    class Style {
        public:
            enum Type : glui32 {
                STYLE(Normal),
                STYLE(Emphasized),
                STYLE(Preformatted),
                STYLE(Header),
                STYLE(Subheader),
                STYLE(Alert),
                STYLE(Note),
                STYLE(BlockQuote),
                STYLE(Input),
                STYLE(User1),
                STYLE(User2)
            };
    };
}

#undef STYLE

#endif
