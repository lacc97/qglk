#include "window.hpp"

#include <cassert>

#include "qglk.hpp"

#include "pairwindow.hpp"

#include "log/log.hpp"

QString Glk::Window::windowsTypeString(glui32 type) {
    switch(type) {
        case Blank:
            return QStringLiteral("Blank");
        case Graphics:
            return QStringLiteral("Graphics");
        case Pair:
            return QStringLiteral("Pair");
        case TextBuffer:
            return QStringLiteral("Text Buffer");
        case TextGrid:
            return QStringLiteral("Text Grid");
        default:
            return QStringLiteral("Unknown");
    }
}

Glk::Window::Window(QIODevice* device_, glui32 rock_, bool acceptsCharRequest, bool acceptsLineRequest, bool acceptsMouseRequest, bool acceptsHyperlinkRequest) : QWidget(NULL), Object(rock_), mp_Parent(NULL), mp_Stream(new WindowStream(this, device_)), mp_KIProvider(new KeyboardInputProvider(this, acceptsCharRequest, acceptsLineRequest)), mp_MIProvider(new MouseInputProvider(this, acceptsMouseRequest)), mp_HLProvider(new HyperlinkInputProvider(this, acceptsHyperlinkRequest)) {
    assert(device_);

    mp_Stream->open(QIODevice::WriteOnly);

    Glk::Dispatch::registerObject(this);
    QGlk::getMainWindow().windowList().append(this);
}

Glk::Window::~Window() {
    //     delete mp_Stream; // deleted by qobject

    if(!QGlk::getMainWindow().windowList().removeOne(this))
        log_warn() << "Window " << (this) << " not found in window list while removing";
    else
        log_trace() << "Window " << (this) << " removed from window list";

    Glk::Dispatch::unregisterObject(this);
}

QSize Glk::Window::windowSize() const {
    return pixelsToUnits(size());
}

void Glk::Window::setWindowParent(Glk::PairWindow* prnt) {
    setParent(prnt ? static_cast<QWidget*>(prnt) : static_cast<QWidget*>(&QGlk::getMainWindow()));
    mp_Parent = prnt;
}

void Glk::Window::keyPressEvent(QKeyEvent* event) {
    int key = event->key();
    QString text(event->text().data());
    
    Glk::postTaskToGlkThread([=]() {
        mp_KIProvider->handleKeyEvent(key, text);
    });
}

void Glk::Window::mouseReleaseEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    
    Glk::postTaskToGlkThread([=]() {
        mp_MIProvider->handleMouseEvent(pos);
    });
}
