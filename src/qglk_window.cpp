#include "window/window.hpp"

#include <sstream>

#include <QDebug>
#include <QSet>

#include "qglk.hpp"

#include "blorb/chunk.hpp"
#include "log/log.hpp"
#include "thread/taskrequest.hpp"
#include "window/pairwindow.hpp"
#include "window/textbufferwindow.hpp"

winid_t glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock) {
    spdlog::trace("glk_window_open({}, {}, {}, {}, {})", wrap::ptr(split), Glk::WindowArrangement::methodString(method), size,
                  Glk::Window::windowsTypeString(wintype), rock);
//    log_trace() << printTree(QGlk::getMainWindow().rootWindow());

    auto windowsType = static_cast<Glk::Window::Type>(wintype);

    switch(windowsType) {
        case Glk::Window::Blank:
        case Glk::Window::Graphics:
        case Glk::Window::TextBuffer:
        case Glk::Window::TextGrid:
            break;

        default:
            spdlog::warn("Tried to open window of type {}", Glk::Window::windowsTypeString(wintype));
            spdlog::trace("glk_window_open({}, {}, {}, {}, {}) => {}", wrap::ptr(split),
                          Glk::WindowArrangement::methodString(method), size, Glk::Window::windowsTypeString(wintype),
                          rock, wrap::ptr((winid_t) (nullptr)));
            return NULL;
    }

    if(!split && !QGlk::getMainWindow().windowList().empty()) {
        spdlog::warn("Tried to open another root window");
        spdlog::trace("glk_window_open({}, {}, {}, {}, {}) => {}", wrap::ptr(split), Glk::WindowArrangement::methodString(method),
                      size, Glk::Window::windowsTypeString(wintype), rock, wrap::ptr((winid_t) (nullptr)));
        return NULL;
    }

    Glk::Window* splitWin = FROM_WINID(split);
    Glk::Window* newWin = Glk::WindowController::createController(windowsType, nullptr, rock)->window();

    if(splitWin) {
        Glk::PairWindow* splitParent = splitWin->parent();
        Glk::PairWindow* pairWindow = Glk::PairWindowController::createController(newWin, newWin, splitWin,
                                                                                  Glk::WindowArrangement::fromMethod(
                                                                                      method, size),
                                                                                  splitParent)->window<Glk::PairWindow>();

        if(splitParent)
            splitParent->replaceChild(splitWin, pairWindow);
        else
            QGlk::getMainWindow().setRootWindow(pairWindow);

    } else {
        QGlk::getMainWindow().setRootWindow(newWin);
    }

    spdlog::trace("glk_window_open({}, {}, {}, {}, {}) => {}", wrap::ptr(split), Glk::WindowArrangement::methodString(method),
                  size, Glk::Window::windowsTypeString(wintype), rock, wrap::ptr(newWin));

    return TO_WINID(newWin);
}

void glk_window_close(winid_t win, stream_result_t* result) {
    SPDLOG_TRACE("glk_window_close({}, {})", wrap::ptr(win), (void*)result);

    Glk::Window* deadWin = FROM_WINID(win);
    Glk::PairWindow* winParent = deadWin->parent();

    if(winParent) {
        Glk::Window* orphanSibling = winParent->removeChild(deadWin);
        Glk::PairWindow* winGrandparent = winParent->parent();

        if(winGrandparent)
            winGrandparent->replaceChild(winParent, orphanSibling);
        else
            QGlk::getMainWindow().setRootWindow(orphanSibling);

        winParent->replaceChild(deadWin, nullptr);
        winParent->replaceChild(orphanSibling, nullptr);
    } else {
        QGlk::getMainWindow().setRootWindow(nullptr);
    }

    QGlk::getMainWindow().eventQueue().cleanWindowEvents(win);

    if(result) {
        result->readcount = deadWin->stream()->readCount();
        result->writecount = deadWin->stream()->writeCount();
    }

    deadWin->controller()->closeWindow();

    /* close windows from bottom to top */
    if(winParent)
        winParent->controller()->closeWindow();
}

void glk_window_get_size(winid_t win, glui32* widthptr, glui32* heightptr) {
    auto size = FROM_WINID(win)->size();

    if(widthptr)
        *widthptr = size.width();

    if(heightptr)
        *heightptr = size.height();
}

void glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin) {
    SPDLOG_TRACE("glk_window_set_arrangement({}, {}, {}, {})", wrap::ptr(win), Glk::WindowArrangement::methodString(method), size, wrap::ptr(keywin));

    if(FROM_WINID(win)->windowType() != Glk::Window::Pair) {
        spdlog::warn("Cannot set arrangement of window {} (must be of type {})", wrap::ptr(win), Glk::Window::windowsTypeString(Glk::Window::Pair));

        return;
    }

    auto pairWindow = static_cast<Glk::PairWindow*>(FROM_WINID(win));

    pairWindow->setArrangement(!keywin ? pairWindow->keyWindow() : FROM_WINID(keywin),
                               Glk::WindowArrangement::fromMethod(method, size));
}

void glk_window_get_arrangement(winid_t win, glui32* methodptr, glui32* sizeptr, winid_t* keywinptr) {
    SPDLOG_TRACE("glk_window_get_arrangement({}, {}, {}, {})", wrap::ptr(win), (void*)methodptr, (void*)sizeptr, (void*)keywinptr);

    if(FROM_WINID(win)->windowType() != Glk::Window::Pair) {
        spdlog::warn("Cannot get arrangement of window {} (must be of type {})", wrap::ptr(win), Glk::Window::windowsTypeString(Glk::Window::Pair));

        return;
    }

    auto pw = static_cast<Glk::PairWindow*>(FROM_WINID(win));

    if(methodptr)
        *methodptr = pw->arrangement()->method();

    if(sizeptr)
        *sizeptr = pw->arrangement()->size();

    if(keywinptr)
        *keywinptr = TO_WINID(pw->keyWindow());
}

void glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos) {
    SPDLOG_TRACE("glk_window_move_cursor({}, {}, {})", wrap::ptr(win), xpos, ypos);

    FROM_WINID(win)->moveCursor(xpos, ypos);
}

void glk_window_set_echo_stream(winid_t win, strid_t str) {
    SPDLOG_TRACE("glk_window_set_echo_stream({}, {})", wrap::ptr(win), wrap::ptr(str));

    FROM_WINID(win)->stream()->setEchoStream(FROM_STRID(str));
}

strid_t glk_window_get_echo_stream(winid_t win) {
    strid_t str = TO_STRID(FROM_WINID(win)->stream()->echoStream());

    SPDLOG_TRACE("glk_window_get_echo_stream({}) => {}", wrap::ptr(win), wrap::ptr(str));

    return str;
}

winid_t glk_window_iterate(winid_t win, glui32* rockptr) {
    const auto& winList = QGlk::getMainWindow().windowList();

    if(win == NULL) {
        if(winList.empty()) {
            SPDLOG_TRACE("glk_window_iterate({}, {}) => {}", wrap::ptr(win), wrap::ptr(rockptr), wrap::ptr(strid_t(NULL)));
            return NULL;
        }

        auto first = winList.front();

        if(rockptr)
            *rockptr = first->rock();

        SPDLOG_TRACE("glk_window_iterate({}, {}) => {}", wrap::ptr(win), wrap::ptr(rockptr), wrap::ptr(first));

        return TO_WINID(first);
    }

    auto it = winList.cbegin();

    while(it != winList.cend() && (*it++) != FROM_WINID(win));

    if(it == winList.cend()) {
        SPDLOG_TRACE("glk_window_iterate({}, {}) => {}", wrap::ptr(win), wrap::ptr(rockptr), wrap::ptr(strid_t(NULL)));
        return NULL;
    }

    if(rockptr)
        *rockptr = (*it)->rock();

    SPDLOG_TRACE("glk_window_iterate({}, {}) => {}", wrap::ptr(win), wrap::ptr(rockptr), wrap::ptr(*it));

    return TO_WINID(*it);
}

glui32 glk_window_get_rock(winid_t win) {
    return FROM_WINID(win)->rock();
}

glui32 glk_window_get_type(winid_t win) {
    return FROM_WINID(win)->windowType();
}

winid_t glk_window_get_parent(winid_t win) {
    return TO_WINID(FROM_WINID(win)->parent());
}

winid_t glk_window_get_sibling(winid_t win) {
    SPDLOG_TRACE("glk_window_get_sibling({})", wrap::ptr(win));

    if(!FROM_WINID(win)->parent()) {
        spdlog::warn("Cannot return sibling of window {} (root window has no siblings)", wrap::ptr(win));

        return NULL;
    }

    Glk::PairWindow* parent = FROM_WINID(win)->parent();
    winid_t sibling = NULL;

    if(parent->keyWindow() == FROM_WINID(win))
        sibling = TO_WINID(parent->secondWindow());
    else if(parent->secondWindow() == FROM_WINID(win))
        sibling = TO_WINID(parent->keyWindow());

    return sibling;
}

winid_t glk_window_get_root() {
    winid_t rw = TO_WINID(QGlk::getMainWindow().rootWindow());

    SPDLOG_TRACE("glk_window_get_root() => {}", wrap::ptr(rw));

    return rw;
}

void glk_window_clear(winid_t win) {
    SPDLOG_TRACE("glk_window_clear({})", wrap::ptr(win));

    FROM_WINID(win)->clearWindow();
}

strid_t glk_window_get_stream(winid_t win) {
    return TO_STRID(FROM_WINID(win)->stream());
}

void glk_set_window(winid_t win) {
    if(!win)
        glk_stream_set_current(NULL);
    else
        glk_stream_set_current(TO_STRID(FROM_WINID(win)->stream()));
}

void glk_window_set_background_color(winid_t win, glui32 color) {
    SPDLOG_TRACE("glk_window_set_background_color({}, {:0x})", wrap::ptr(win), color);

    FROM_WINID(win)->setBackgroundColor(QColor::fromRgb(color));
}

void glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    SPDLOG_TRACE("glk_window_fill_rect({}, {:0x}, {}, {}, {}, {})", wrap::ptr(win), color, left, top, width, height);

    FROM_WINID(win)->fillRect(QColor::fromRgb(color), QRect(left, top, width, height));
}

void glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    FROM_WINID(win)->eraseRect(QRect(left, top, width, height));
}

// style related functions use threads because in the future it should be possible to change
// fonts and colours from QGlk via a menu or something
void glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val) {
    if(wintype != Glk::Window::TextBuffer)
        return;

    QGlk::getMainWindow().textBufferStyleManager()[static_cast<Glk::Style::Type>(styl)].setHint(hint, val);
}

void glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint) {
    if(wintype != Glk::Window::TextBuffer)
        return;

    QGlk::getMainWindow().textBufferStyleManager()[static_cast<Glk::Style::Type>(styl)].setHint(hint,
                                                                                                QGlk::getMainWindow().defaultStyleManager()[static_cast<Glk::Style::Type>(styl)].getHint(
                                                                                                    hint));
}

glui32 glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextBuffer)
        return FALSE;

    bool res;

    res = static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl1)] !=
          static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl2)];

    return res;
}

glui32 glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32* result) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextBuffer || !result)
        return FALSE;

    bool res;

    res = static_cast<Glk::TextBufferWindow*>(FROM_WINID(
        win))->styles()[static_cast<Glk::Style::Type>(styl)].measureHint(hint, result);

    return res;
}

glui32 glk_image_get_info(glui32 image, glui32* width, glui32* height) {
    Glk::Blorb::Chunk imgchunk = Glk::Blorb::loadResource(image, Glk::Blorb::ResourceUsage::Picture);

    if(!imgchunk.isValid())
        return FALSE;

    QImage img = QImage::fromData(reinterpret_cast<const uchar*>(imgchunk.data()), imgchunk.length());

    if(img.isNull())
        return FALSE;

    if(width)
        *width = img.width();

    if(height)
        *height = img.height();

    return TRUE;
}

glui32 glk_image_draw(winid_t win, glui32 image, glsi32 val1, glsi32 val2) {
    return (FROM_WINID(win)->drawImage(image, val1, val2, {}) ? TRUE : FALSE);
}

glui32 glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height) {
    return (FROM_WINID(win)->drawImage(image, val1, val2, QSize(width, height)) ? TRUE : FALSE);
}

void glk_window_flow_break(winid_t win) {
    FROM_WINID(win)->flowBreak();
}
