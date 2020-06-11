#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

#include <list>
#include <memory>
#include <vector>

#include <QLinkedList>
#include <QTextBrowser>

#include "stylemanager.hpp"
#include "textbufferwindowcontroller.hpp"
#include "window.hpp"

namespace Glk {
    class TextBufferWindow;

    class TextBufferBuf : public WindowBuf {
        public:
            explicit TextBufferBuf(TextBufferWindow* win);

        protected:
            std::streamsize xsputn(const char_type* s, std::streamsize count) final;
    };

    class TextBufferWindow : public Window {
            friend class TextBufferBuf;

        public:
            TextBufferWindow(TextBufferWindowController* winController, PairWindow* winParent, glui32 winRock);

            ~TextBufferWindow() final = default;


            void clearWindow() override;

            bool drawImage(const QImage& img, glsi32 param1, glsi32 param2, QSize imgSize) override;

            void flowBreak() override;

            void pushHyperlink(glui32 linkValue) override;

            void pushStyle(Glk::Style::Type style) override;


            void writeString(const QString& str);


            [[nodiscard]] inline const std::vector<QImage>& images() const {
                return m_Images;
            }

            [[nodiscard]] inline const StyleManager& styles() const {
                return m_Styles;
            }

        private:
            std::vector<QImage> m_Images;
            StyleManager m_Styles;
            Style::Type m_CurrentStyleType;
            QTextBlockFormat m_CurrentBlockFormat;
            QTextCharFormat m_CurrentCharFormat;
            QTextCharFormat m_NonHyperlinkCharFormat;
            glui32 m_CurrentHyperlink;
    };
}

#endif
