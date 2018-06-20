#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

#include "window.hpp"

// #include <QLabel>
#include <QTextBrowser>

namespace Glk {
    class TextBufferWindow;

    class TextBufferDevice : public QIODevice {
        public:
            TextBufferDevice(TextBufferWindow* win);

            qint64 readData(char* data, qint64 maxlen) override;
            qint64 writeData(const char* data, qint64 len) override;

        private:
            TextBufferWindow* mp_TBWindow;
    };

    class TextBufferWindow : public Window {
            friend class TextBufferDevice;

            Q_OBJECT
        public:
            TextBufferWindow(glui32 rock_ = 0);
            ~TextBufferWindow() {}

            Glk::Window::Type windowType() const override {
                return Window::TextBuffer;
            }

            void clearWindow() override;

        public slots:
            void onTextChanged();

        protected:
            void resizeEvent(QResizeEvent* ev) override;

            QSize pixelsToUnits(const QSize& pixels) const override;
            QSize unitsToPixels(const QSize& units) const override;

        protected slots:
            void onCharacterInputRequested();
            void onCharacterInputRequestEnded(bool cancelled);
            void onLineInputRequested();
            void onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode);
            
            void onCharacterInput(glui32 ch);
            void onSpecialCharacterInput(glui32 ch);

        private:
            QTextBrowser* mp_Text;
//         QLabel* mp_Text;
    };
}

#endif
