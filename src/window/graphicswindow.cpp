#include "graphicswindow.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include "stream/nulldevice.hpp"

Glk::GraphicsWindow::GraphicsWindow(glui32 rock_) : Window(new NullDevice(), rock_, true, false), m_Buffer(QSize(1, 1), QImage::Format_ARGB32) {
    setBackgroundColor(Qt::white);
}

void Glk::GraphicsWindow::setBackgroundColor(const QColor& c) {
    QPalette pal = palette();
    pal.setColor(QPalette::Background, c);
    setAutoFillBackground(true);
    setPalette(pal);
}

bool Glk::GraphicsWindow::drawImage(const QImage& im, glsi32 x, glsi32 y, glui32 w, glui32 h) {
    QPainter p(&m_Buffer);
    p.drawImage(QRect(x, y, w, h), im);
    return true;
}

void Glk::GraphicsWindow::fillRect(const QColor& c, glsi32 x, glsi32 y, glui32 w, glui32 h) {
    QPainter p(&m_Buffer);
    p.fillRect(x, y, w, h, c);
}

void Glk::GraphicsWindow::clearWindow() {
    m_Buffer = QImage(size(), QImage::Format_ARGB32);
}

void Glk::GraphicsWindow::paintEvent(QPaintEvent* ev) {
    QRect r = ev->region().boundingRect();

    QPainter p(this);
//     p.fillRect(event->region().boundingRect(), palette().background());
    p.drawImage(r, m_Buffer, r);
}

void Glk::GraphicsWindow::resizeEvent(QResizeEvent* ev) {
    QImage newi(ev->size(), QImage::Format_ARGB32);

    QPainter* p = new QPainter(&newi);
//     p->fillRect(rect(), Qt::transparent);
    p->drawImage(QPoint(0, 0), m_Buffer);
    delete p;

    m_Buffer = std::move(newi);
}

QSize Glk::GraphicsWindow::pixelsToUnits(const QSize& pixels) const {
    return pixels;
}

QSize Glk::GraphicsWindow::unitsToPixels(const QSize& units) const {
    return units;
}
