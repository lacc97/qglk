#ifndef GRAPHICSWINDOW_HPP
#define GRAPHICSWINDOW_HPP

#include "window.hpp"

#include <QColor>
#include <QPixmap>

namespace Glk {
    class GraphicsWindow : public Window {
            Q_OBJECT
        public:
            GraphicsWindow(glui32 rock_ = 0);
            ~GraphicsWindow() {}

            void setBackgroundColor(const QColor& c);

            bool drawImage(const QPixmap& im, glsi32 x, glsi32 y, glui32 w, glui32 h);
            void fillRect(const QColor& c, glsi32 x, glsi32 y, glui32 w, glui32 h);

            Glk::Window::Type windowType() const override {
                return Window::Graphics;
            }

            void clearWindow() override;

        protected:
            void paintEvent(QPaintEvent* ev) override;
            void resizeEvent(QResizeEvent* ev) override;
            
            QSize pixelsToUnits(const QSize & pixels) const override;
            QSize unitsToPixels(const QSize & units) const override;

        private:
            QPixmap m_Buffer;
            QPixmap m_BackBuffer;
    };
}

#endif
