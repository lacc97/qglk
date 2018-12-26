#include "graphicswindow.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include "qglk.hpp"
#include "stream/nulldevice.hpp"

Glk::GraphicsWindow::GraphicsWindow(glui32 rock_) : Window(new NullDevice(), rock_, true, false, true, false), m_Buffer(QSize(1, 1)) {
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::white);
    setPalette(pal);
    
    connect(
        &QGlk::getMainWindow(), &QGlk::poll,
        this, qOverload<>(&Glk::GraphicsWindow::update));
}

void Glk::GraphicsWindow::setBackgroundColor(const QColor& c) {
    QPalette pal = palette();
    QColor prev = pal.color(QPalette::Background);
    pal.setColor(QPalette::Background, c);
    setPalette(pal);
    
    if(prev != c)
        QGlk::getMainWindow().eventQueue().push(event_t{evtype_Redraw, TO_WINID(this), 0, 0});
}

bool Glk::GraphicsWindow::drawImage(const QPixmap& im, glsi32 x, glsi32 y, glui32 w, glui32 h) {
    QPainter* p = new QPainter(&m_Buffer);
    p->drawPixmap(QRect(x, y, w, h), im);
    delete p;
    
    return true;
}

void Glk::GraphicsWindow::fillRect(const QColor& c, glsi32 x, glsi32 y, glui32 w, glui32 h) {
    QPainter* p = new QPainter(&m_Buffer);
    p->fillRect(x, y, w, h, c);
    delete p;
}

void Glk::GraphicsWindow::clearWindow() {
    m_Buffer = QPixmap(size());
    
    QPainter* p = new QPainter(&m_Buffer);
    p->fillRect(rect(), palette().background());
    delete p;
}

void Glk::GraphicsWindow::paintEvent(QPaintEvent* ev) {
//     Window::paintEvent(ev);
    
    QRect r = ev->region().boundingRect();

    QPainter* p = new QPainter(this);
    p->drawPixmap(r, m_Buffer, r);
    delete p;
}

void Glk::GraphicsWindow::resizeEvent(QResizeEvent* ev) {
//     Window::resizeEvent(ev);
    
    QPixmap newi(ev->size());

    QPainter* p = new QPainter(&newi);
    p->fillRect(rect(), palette().background());
    p->drawPixmap(QPoint(0, 0), m_Buffer);
    delete p;

    m_Buffer = std::move(newi);
    
//     QGlk::getMainWindow().eventQueue().push(event_t{evtype_Redraw, TO_WINID(this), 0, 0});
}

QSize Glk::GraphicsWindow::pixelsToUnits(const QSize& pixels) const {
    return pixels;
}

QSize Glk::GraphicsWindow::unitsToPixels(const QSize& units) const {
    return units;
}
