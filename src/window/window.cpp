#include "window.hpp"

#include <cassert>

#include "log/log.hpp"

#include "qglk.hpp"

#include "window_stream_driver.hpp"

Glk::Window::Window(Type type, WindowController* winController, std::unique_ptr<qglk::stream_drivers::window> streambuf, PairWindow* winParent, glui32 rock)
    : Object{rock},
      m_Type{type},
      mp_Controller{winController},
      mp_Stream{new qglk::stream{0}},
      mp_Parent{winParent} {
    assert(mp_Controller);
    assert(mp_Stream);

    mp_Stream->init(glk_stream_struct::eWindow, true, true, std::move(streambuf));

    QGlk::getMainWindow().dispatch().registerObject(this);
    QGlk::getMainWindow().windowList().push_back(this);
    SPDLOG_DEBUG("{} appended to window list", *this);
}

Glk::Window::~Window() {
    QGlk::getMainWindow().eventQueue().cleanWindowEvents(TO_WINID(this));

    auto& winList = QGlk::getMainWindow().windowList();
    if(std::count(winList.begin(), winList.end(), this) == 0) {
        spdlog::warn("{} not found in window list while removing", *this);
    } else {
        winList.remove(this);
        SPDLOG_DEBUG("{} removed from window list", *this);
    }

    QGlk::getMainWindow().dispatch().unregisterObject(this);
}

bool Glk::Window::drawImage(glui32 img, glsi32 param1, glsi32 param2, QSize size) {
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
