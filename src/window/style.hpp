#ifndef WINDOW_STYLE_HPP
#define WINDOW_STYLE_HPP

#include "glk.hpp"

#include <QColor>
#include <QFont>

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

            Style(Type type_ = Normal);
            
            QString styleString() const;

            glui32 getHint(glui32 hint) const;
            bool measureHint(glui32 hint, glui32* result) const;
            void setHint(glui32 hint, glui32 value);
            inline Type type() const {
                return m_Type;
            }

            bool operator==(const Style& other) const;
            inline bool operator!=(const Style& other) const {
                return !(operator==(other));
            }

        private:
            Type m_Type;
            QFont m_Font;
            glsi32 m_Indentation;
            glsi32 m_ParaIndentation;
            glui32 m_Justification;
            glsi32 m_FontSizeIncrease;
            QColor m_TextColor;
            QColor m_BackgroundColor;

    };
}

#undef STYLE

#endif
