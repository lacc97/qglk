#ifndef GRAPHICSWINDOW_HPP
#define GRAPHICSWINDOW_HPP

#include <QColor>
#include <QImage>

#include "graphicswindowcontroller.hpp"
#include "window.hpp"

namespace Glk {
    class GraphicsWindow : public Window {
        public:
            GraphicsWindow(GraphicsWindowController* winController, PairWindow* winParent, glui32 objRock = 0);

            ~GraphicsWindow() final = default;


            void clearWindow() override;

            bool drawImage(glui32 image, glsi32 param1, glsi32 param2, QSize size) override;

            void eraseRect(const QRect& rect) override;

            void fillRect(const QColor& color, const QRect& rect) override;

            void setBackgroundColor(const QColor& color) override;


            [[nodiscard]] inline const QColor& backgroundColor() const {
                return m_BGColor;
            }

            [[nodiscard]] inline const QImage& buffer() const {
                return m_Buffer;
            }

            void resizeBuffer(QSize newSize);

        private:
            QImage m_Buffer;
            QColor m_BGColor;
    };
}

#endif
