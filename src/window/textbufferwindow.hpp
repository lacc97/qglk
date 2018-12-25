#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

// #include <QLabel>
#include <QTextBrowser>

#include "stylemanager.hpp"
#include "window.hpp"

namespace Glk {
    class TextBufferWindow;

    class TextBufferDevice : public QIODevice {
            Q_OBJECT
        public:
            TextBufferDevice(TextBufferWindow* win);

            qint64 readData(char* data, qint64 maxlen) override;
            qint64 writeData(const char* data, qint64 len) override;

        public slots:
            void discard();
            void flush();
            void onWindowStyleChanged(const QString& newStyleString);

        signals:
            void textChanged();

        private:
            TextBufferWindow* mp_TBWindow;
            QString m_StyleString;
            QStringList m_Buffer;
    };

    class TextBufferWindow : public Window {
            Q_OBJECT

            friend class TextBufferDevice;
        public:
            TextBufferWindow(glui32 rock_ = 0);
            ~TextBufferWindow() {}

            inline const Glk::StyleManager& styles() const {
                return m_Styles;
            }

            inline Glk::Style::Type getStyle() const {
                return m_CurrentStyleType;
            }
            void setStyle(Glk::Style::Type style) override;

            Glk::Window::Type windowType() const override {
                return Window::TextBuffer;
            }

            void clearWindow() override;

        public slots:
            void onTextChanged();

        signals:
            void styleChanged(const QString& newStyleString);

        protected:
            inline Glk::TextBufferDevice* ioDevice() const {
                return static_cast<Glk::TextBufferDevice*>(windowStream()->getIODevice());
            }
            
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
            Glk::StyleManager m_Styles;
            Glk::Style::Type m_CurrentStyleType;
            Glk::Style::Type m_PreviousStyleType;
    };
}

#endif
