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

Glk::Window::Window(Type type, WindowController* winController, WindowDevice* streamDevice, PairWindow* winParent, glui32 rock)
    : Object{rock},
      m_Type{type},
      mp_Controller{winController},
      mp_Stream{new WindowStream{streamDevice}},
      mp_Parent{winParent} {
    assert(mp_Controller);
    assert(mp_Stream);

    mp_Stream->open(QIODevice::WriteOnly);

    QGlk::getMainWindow().dispatch().registerObject(this);
    QGlk::getMainWindow().windowList().push_back(this);
    SPDLOG_DEBUG("{} appended to window list", *this);
}

Glk::Window::~Window() {
    auto& winList = QGlk::getMainWindow().windowList();
    if(std::count(winList.begin(), winList.end(), this) == 0) {
        spdlog::warn("{} not found in window list while removing", *this);
    } else {
        winList.remove(this);
        SPDLOG_DEBUG("{} removed from window list", *this);
    }

    QGlk::getMainWindow().dispatch().unregisterObject(this);
}

bool Glk::Window::drawImage(const QImage& img, glsi32 param1, glsi32 param2, QSize imgSize) {
    spdlog::warn("Cannot draw image in window {}", *this);
    return false;
}

void Glk::Window::eraseRect(const QRect& rect) {
    spdlog::warn("Cannot erase rect in window {}", *this);
}

void Glk::Window::fillRect(const QColor& color, const QRect& rect) {
    spdlog::warn("Cannot fill rect in window {}", *this);
}

void Glk::Window::flowBreak() {
    spdlog::warn("Cannot break flow in window {}", *this);
}

void Glk::Window::moveCursor(glui32 x, glui32 y) {
    spdlog::warn("Cannot move cursor of window {}", *this);
}

void Glk::Window::pushHyperlink(glui32 linkValue) {
    spdlog::warn("Cannot push hyperlink to window {}", *this);
}

void Glk::Window::pushStyle(Glk::Style::Type style) {
    spdlog::warn("Cannot push style to window {}", *this);
}

void Glk::Window::setBackgroundColor(const QColor& color) {
    spdlog::warn("Cannot change background colour of window {}", *this);
}

std::ostream& operator<<(std::ostream& os, Glk::Window* win) {
    if(win)
        return os << Glk::Window::windowsTypeString(win->windowType()).toStdString() << "(" << ((void*)win) << ")";
    else
        return os << "null";
}
