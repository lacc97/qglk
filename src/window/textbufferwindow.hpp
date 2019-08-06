#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

#include <memory>
#include <vector>

#include <QLinkedList>
#include <QTextBrowser>

#include "stylemanager.hpp"
#include "textbufferwindowcontroller.hpp"
#include "window.hpp"

namespace Glk {
    class TextBufferWindow;

    class TextBufferDevice : public WindowDevice {
        public:
            explicit TextBufferDevice(TextBufferWindow* win);

        protected:
            qint64 readData(char* data, qint64 maxlen) override;

            qint64 writeData(const char* data, qint64 len) override;
    };

    class TextBufferWindow : public Window {
            friend class TextBufferDevice;

            class History {
                    typedef QLinkedList<QString> linked_list_type;

                    static constexpr int MAX_SIZE = 1000;
                public:
                    History();

                    void push(const QString& newcmd);

                    void resetIterator();

                    const QString next();

                    const QString previous();

                private:
                    linked_list_type m_History;
                    linked_list_type::iterator m_Iterator;
            };

        public:
            TextBufferWindow(TextBufferWindowController* winController, PairWindow* winParent, glui32 winRock);

            ~TextBufferWindow() final = default;


            void clearWindow() override;

            bool drawImage(const QImage& img, glsi32 param1, glsi32 param2, QSize imgSize) override;

            void flowBreak() override;

            void pushHyperlink(glui32 linkValue) override;

            void pushStyle(Glk::Style::Type style) override;

            [[nodiscard]] Window::Type windowType() const override {
                return TextBuffer;
            }


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
