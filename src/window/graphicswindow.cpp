#include "graphicswindow.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

#include "qglk.hpp"
#include "stream/nulldevice.hpp"

Glk::GraphicsWindow::GraphicsWindow(GraphicsWindowController* winController, PairWindow* winParent, glui32 objRock)
    : Window(Type::Graphics, winController, std::make_unique<WindowBuf>(this), winParent, objRock),
      m_Buffer{1, 1, QImage::Format_ARGB32},
      m_BGColor{} {
}

void Glk::GraphicsWindow::clearWindow() {
    assert(onGlkThread());

    m_Buffer.fill(Qt::transparent);

    controller()->requestSynchronization();
}

bool Glk::GraphicsWindow::drawImage(const QImage& img, glsi32 param1, glsi32 param2, QSize imgSize) {
    assert(onGlkThread());

    std::unique_ptr<QPainter> p = std::make_unique<QPainter>(&m_Buffer);
    p->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    p->drawImage(QRect{param1, param2, imgSize.width(), imgSize.height()}, img);

    controller()->requestSynchronization();

    return true;
}

void Glk::GraphicsWindow::eraseRect(const QRect& rect) {
    fillRect(Qt::transparent, rect);
}

void Glk::GraphicsWindow::fillRect(const QColor& color, const QRect& rect) {
    assert(onGlkThread());

    std::unique_ptr<QPainter> p = std::make_unique<QPainter>(&m_Buffer);
    p->fillRect(rect, color);

    controller()->requestSynchronization();
}

void Glk::GraphicsWindow::setBackgroundColor(const QColor& color) {
    assert(onGlkThread());

    m_BGColor = color;
}

void Glk::GraphicsWindow::resizeBuffer(QSize newSize) {
    assert(onEventThread());

    QImage newBuffer{newSize, QImage::Format_ARGB32};
    newBuffer.fill(Qt::transparent);

    QRect dstRect = newBuffer.rect();
    QRect srcRect = {0, 0,
                     std::min(m_Buffer.width(), newBuffer.width()),
                     std::min(m_Buffer.height(), newBuffer.height())};

    std::unique_ptr<QPainter> p = std::make_unique<QPainter>(&newBuffer);
    p->drawImage(dstRect, m_Buffer, srcRect);

    m_Buffer = std::move(newBuffer);
}
