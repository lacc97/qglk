#include "window.hpp"

#include <cassert>

#include "log/log.hpp"

#include "window/blankwindow.hpp"
#include "window/graphicswindow.hpp"
#include "window/pairwindow.hpp"
#include "window/textbufferwindow.hpp"
#include "window/textgridwindow.hpp"

#include "qglk.hpp"


QString Glk::Window::windowsTypeString(glui32 type) {
    switch(type) {
        case Blank:
            return QStringLiteral("Blank");
        case Graphics:
            return QStringLiteral("Graphics");
        case Pair:
            return QStringLiteral("Pair");
        case TextBuffer:
            return QStringLiteral("TextBuffer");
        case TextGrid:
            return QStringLiteral("TextGrid");
        default:
            return QStringLiteral("Unknown");
    }
}

Glk::Window::Window(WindowController* winController, WindowDevice* streamDevice, PairWindow* winParent, glui32 rock)
    : Object{rock},
      mp_Controller{winController},
      mp_Stream{new WindowStream{streamDevice}},
      mp_Parent{winParent} {
    assert(mp_Controller);
    assert(mp_Stream);

    mp_Stream->open(QIODevice::WriteOnly);

    Dispatch::registerObject(this);
}

Glk::Window::~Window() {
    if(!QGlk::getMainWindow().windowList().removeOne(this))
        log_warn() << (this) << " not found in window list while removing";
    else
        log_trace() << (this) << " removed from window list";

    Dispatch::unregisterObject(this);
}

bool Glk::Window::drawImage(const QImage& img, glsi32 param1, glsi32 param2, QSize imgSize) {
    log_warn() << "Cannot draw image in window " << this;

    return false;
}

void Glk::Window::eraseRect(const QRect& rect) {
    log_warn() << "Cannot erase rect in window " << this;
}

void Glk::Window::fillRect(const QColor& color, const QRect& rect) {
    log_warn() << "Cannot fill rect in window " << this;
}

void Glk::Window::flowBreak() {
    log_warn() << "Cannot break flow in window " << this;
}

void Glk::Window::moveCursor(glui32 x, glui32 y) {
    log_warn() << "Cannot move cursor of window " << this;
}

void Glk::Window::pushHyperlink(glui32 linkValue) {
    log_warn() << "Cannot push hyperlink to window " << this;
}

void Glk::Window::pushStyle(Glk::Style::Type style) {
    log_warn() << "Cannot push style to window " << this;
}

void Glk::Window::setBackgroundColor(const QColor& color) {
    log_warn() << "Cannot change background colour of window " << this;
}

std::ostream& operator<<(std::ostream& os, Glk::Window* win) {
    if(win)
        return os << Glk::Window::windowsTypeString(win->windowType()).toStdString() << "(" << ((void*)win) << ")";
    else
        return os << "null";
}
