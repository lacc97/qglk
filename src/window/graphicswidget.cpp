#include "graphicswidget.hpp"

#include <memory>

#include <QPainter>
#include <QPaintEvent>

Glk::GraphicsWidget::GraphicsWidget() : WindowWidget{} {
    m_DefaultBackgroundColor = palette().color(QPalette::Window);

    installInputFilter(this);
}

void Glk::GraphicsWidget::setBackgroundColor(const QColor& c) {
    QPalette pal = palette();
    pal.setColor(QPalette::Window, c);
    setPalette(pal);
}

void Glk::GraphicsWidget::paintEvent(QPaintEvent* event) {
    QWidget::paintEvent(event);

    if(!m_Buffer.isNull()) {
        QRect r = event->region().boundingRect();

        std::unique_ptr<QPainter> p = std::make_unique<QPainter>(this);
        p->drawPixmap(r, m_Buffer, r);
    }
}

void Glk::GraphicsWidget::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);

    if(!m_Buffer.isNull())
        m_Buffer = m_Buffer.scaled(event->size());

    emit resized();
}
