#ifndef INPUTPROVIDER_HPP
#define INPUTPROVIDER_HPP

#include "glk.hpp"

#include <QKeyEvent>
#include <QMouseEvent>

namespace Glk {
    class Window;

    class KeyboardInputProvider : public QObject {
            Q_OBJECT
        public:
            KeyboardInputProvider(Glk::Window* parent_, bool characterInputProvider_, bool lineInputProvider_);

            void requestCharInput(bool unicode);
            void cancelCharInputRequest();
            void requestLineInput(void* buf, glui32 maxlen, glui32 initlen, bool unicode);
            void cancelLineInputRequest(event_t* ev);

            bool echoesLine() const;
            void setLineEcho(bool le);

            void setTerminators(glui32* keycodes, glui32 count);
            
            void clearLineInputBuffer();
            void fillLineInputBuffer(const QString& text);

            bool handleKeyEvent(int key, const QString& text);

        signals:
            void characterInputRequested();
            void characterInputRequestEnded(bool cancelled);
            void lineInputRequested();
            void lineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode);

            void lineInputCharacterEntered(glui32 ch, bool doUpdate = true);
            void lineInputSpecialCharacterEntered(glui32 kc, bool doUpdate = true);

        protected:
            Glk::Window* windowPointer();
            
            void processLineInputCharacterEntered(glui32 ch, bool doUpdate = true);

        private:
            bool m_CharacterInputProvider;
            bool m_LineInputProvider;

            bool m_CharacterInputRequested;
            bool m_LineInputRequested;
            bool m_Unicode;

            void* mp_LineInputBuffer;
            glui32 m_LineInputBufferLength;
            glui32 m_LineInputBufferPosition;
            bool m_EchoesLine;
            QSet<glui32> m_Terminators;
    };

    class MouseInputProvider : public QObject {
            Q_OBJECT
        public:
            MouseInputProvider(Glk::Window* parent_, bool m_MouseInputProvider);

            void requestMouseInput();
            void cancelMouseInputRequest();

            bool handleMouseEvent(QPoint pos);

        signals:
            void mouseInputRequested();
            void mouseInputRequestEnded(bool cancelled);

        protected:
            Glk::Window* windowPointer();

        private:
            bool m_MouseInputProvider;
            bool m_MouseInputRequested;
    };

    class HyperlinkInputProvider : public QObject {
            Q_OBJECT

        public:
            HyperlinkInputProvider(Glk::Window* parent_, bool hyperlinkInputProvider_);

            void requestHyperlinkInput();
            void cancelHyperlinkInputRequest();

            void handleHyperlinkClicked(glui32 linkval);

        signals:
            void hyperlinkInputRequested();
            void hyperlinkInputRequestEnded(bool cancelled);
            
        protected:
            Glk::Window* windowPointer();

        private:
            bool m_HyperlinkInputProvider;
            bool m_HyperlinkInputRequested;
    };
}

#endif
