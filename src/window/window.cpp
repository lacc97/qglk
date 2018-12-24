#include "window.hpp"

#include <cassert>

#include "qglk.hpp"

#include "pairwindow.hpp"

Glk::Window::Window(QIODevice* device_, glui32 rock_, bool acceptsCharRequest, bool acceptsLineRequest, bool acceptsMouseRequest) : QWidget(NULL), Object(rock_), mp_Parent(NULL), mp_Stream(new WindowStream(this, device_)), mp_KIProvider(new KeyboardInputProvider(this, acceptsCharRequest, acceptsLineRequest)), mp_MIProvider(new MouseInputProvider(this, acceptsMouseRequest)) {
    assert(device_);

    mp_Stream->open(QIODevice::WriteOnly);

    Glk::Dispatch::registerObject(this);
    s_WindowSet.insert(this);
}

Glk::Window::~Window() {
//     delete mp_Stream; // deleted by qobject

    Glk::Dispatch::unregisterObject(this);
    s_WindowSet.remove(this);
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
