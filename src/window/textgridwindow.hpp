#ifndef TEXTGRIDWINDOW_HPP
#define TEXTGRIDWINDOW_HPP

#include <QIODevice>
#include <QPoint>
#include <QVector>

#include "window.hpp"

namespace Glk {
    class TextGridWindow;

    class TextGridDevice : public QIODevice {
        public:
            TextGridDevice(TextGridWindow* win);

            qint64 readData(char* data, qint64 maxlen) override;
            qint64 writeData(const char* data, qint64 len) override;
            
        private:
            TextGridWindow* mp_TGWindow;
    };

    class TextGridWindow : public Window {
            friend class TextGridDevice;

            Q_OBJECT
        public:
            TextGridWindow(glui32 rock_ = 0);
            ~TextGridWindow() {}

            inline void setGridCursor(glui32 xpos, glui32 ypos) {
                m_Cursor = QPoint(xpos, ypos);
            }

            Glk::Window::Type windowType() const override {
                return Window::TextGrid;
            }

            void clearWindow() override;

        protected:
            void paintEvent(QPaintEvent* event) override;
            void resizeEvent(QResizeEvent* ev) override;

//             void onCharacterInputRequested() override;
//             void onCharacterInputRequestEnded() override;

//             void onCharacterEntered(glui32 ch) override;

            inline Glk::TextGridDevice* ioDevice() const {
                return static_cast<Glk::TextGridDevice*>(windowStream()->getIODevice());
            }

            QSize pixelsToUnits(const QSize& pixels) const override;
            QPoint pixelsToUnits(QPoint pixels) const;
            QSize unitsToPixels(const QSize& units) const override;

            bool writeChar(glui32 ch);
            bool deletePreviousChar();

        protected slots:
            void onCharacterInputRequested();
            void onCharacterInputRequestEnded(bool cancelled);
            void onLineInputRequested();
            void onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode);

            void onCharacterInput(glui32 ch);
            void onSpecialCharacterInput(glui32 ch);

        private:
            QVector<QVector<glui32>> m_CharArray;
            QPoint m_Cursor;
    };
}

#endif
