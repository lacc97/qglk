#include "window.hpp"

#include <cassert>

#include "qglk.hpp"

#include "pairwindow.hpp"

Glk::Window::Window(QIODevice* device_, glui32 rock_, bool acceptsCharRequest, bool acceptsLineRequest, bool acceptsMouseRequest) : Object(rock_), QWidget(NULL), mp_Parent(NULL), mp_Stream(new WindowStream(this, device_)), mp_KIProvider(new KeyboardInputProvider(this, acceptsCharRequest, acceptsLineRequest)), mp_MIProvider(new MouseInputProvider(this, acceptsMouseRequest)) {
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
    Glk::postTaskToGlkThread([&]() {
        mp_KIProvider->handleKeyEvent(event);
    });
}

void Glk::Window::mouseReleaseEvent(QMouseEvent* event) {
    Glk::postTaskToGlkThread([&]() {
        mp_MIProvider->handleMouseEvent(event);
    });
}
