#ifndef GRAPHICSWIDGET_HPP
#define GRAPHICSWIDGET_HPP

#include <QColor>
#include <QPixmap>

#include "windowwidget.hpp"

namespace Glk {
    class GraphicsWidget : public WindowWidget {
        Q_OBJECT
        public:
            GraphicsWidget();


            [[nodiscard]] inline const QColor& backgroundColor() {
                return palette().color(QPalette::Background);
            }

            void setBackgroundColor(const QColor& c);

            inline void setBuffer(const QPixmap& buf) {
                m_Buffer = buf;
            }

            [[nodiscard]] inline const QPixmap& buffer() {
                return m_Buffer;
            }

            [[nodiscard]] inline const QColor& getDefaultBackgroundColor() {
                return m_DefaultBackgroundColor;
            }

        signals:

            void resized();

        protected:
            void paintEvent(QPaintEvent* event) override;

            void resizeEvent(QResizeEvent* event) override;

        private:
            QColor m_DefaultBackgroundColor;
            QPixmap m_Buffer;
    };
}


#endif //GRAPHICSWIDGET_HPP
