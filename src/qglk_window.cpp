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

// #ifndef NDEBUG
std::string printTree(Glk::Window* root) {
    auto fn_print_recursive = [](auto& this_fn, Glk::Window* win, int tabs, std::ostream& os) mutable -> void {
        if(!win)
            return;

        for(int ii = 0; ii < tabs; ii++)
            os << "  ";

        os << win << ": {"
             << "parent <= " << win->parent() << ";";
//             << "children <=" << root->children() << ";"
//             << "layout <=" << root->layout() << ";"
//             << "rect <=" << root->controller()->widget()->rect() << ";";

        if(win->windowType() == Glk::Window::Pair)
            os << "key <= " << static_cast<Glk::PairWindow*>(win)->keyWindow() << ";";

        os << "}\n";

        if(win->windowType() == Glk::Window::Pair) {
            auto pw = static_cast<Glk::PairWindow*>(win);
            this_fn(this_fn, pw->firstWindow(), tabs + 1, os);
            this_fn(this_fn, pw->secondWindow(), tabs + 1, os);
        }
    };

    std::stringstream ss;

    ss << "\n";
    fn_print_recursive(fn_print_recursive, root, 0, ss);

    return ss.str();
}
// #endif

winid_t glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock) {
    log_trace() << "glk_window_open(" << split << ", " << Glk::WindowArrangement::methodString(method) << ", " << size
                << ", " << Glk::Window::windowsTypeString(wintype) << ", " << rock << ")";
    log_trace() << printTree(QGlk::getMainWindow().rootWindow());

    auto windowsType = static_cast<Glk::Window::Type>(wintype);

    switch(windowsType) {
        case Glk::Window::Blank:
        case Glk::Window::Graphics:
        case Glk::Window::TextBuffer:
        case Glk::Window::TextGrid:
            break;

        default:
            log_warn() << "Cannot open window of type " << Glk::Window::windowsTypeString(wintype);
            log_trace() << "glk_window_open(" << split << ", " << Glk::WindowArrangement::methodString(method) << ", "
                        << size << ", " << Glk::Window::windowsTypeString(wintype) << ", " << rock << ") => NULL";
            return NULL;
    }

    if(!split && !QGlk::getMainWindow().windowList().empty()) {
        log_warn() << "Cannot open another root window";
        log_trace() << "glk_window_open(" << split << ", " << Glk::WindowArrangement::methodString(method) << ", "
                    << size << ", " << Glk::Window::windowsTypeString(wintype) << ", " << rock << ") => NULL";
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

    log_trace() << "glk_window_open(" << split << ", "
                << Glk::WindowArrangement::methodString(method) << ", "
                << size << ", " << Glk::Window::windowsTypeString(wintype) << ", "
                << rock << ") => " << TO_WINID(newWin);

    return TO_WINID(newWin);
}

void glk_window_close(winid_t win, stream_result_t* result) {
    log_trace() << "glk_window_close(" << win << ", " << result << ")";
//    log_trace() << printTree(QGlk::getMainWindow().rootWindow());

    Glk::Window* deadWin = FROM_WINID(win);
    Glk::PairWindow* winParent = deadWin->parent();

    if(winParent) {
        Glk::Window* orphanSibling = winParent->removeChild(deadWin);
        Glk::PairWindow* winGrandparent = winParent->parent();

        orphanSibling->setParent(winGrandparent);

        if(winGrandparent)
            winGrandparent->replaceChild(winParent, orphanSibling);
        else
            QGlk::getMainWindow().setRootWindow(orphanSibling);

        winParent->replaceChild(deadWin, nullptr);
        winParent->replaceChild(orphanSibling, nullptr);

        winParent->controller()->closeWindow();
    } else {
        QGlk::getMainWindow().setRootWindow(nullptr);
    }

    QGlk::getMainWindow().eventQueue().cleanWindowEvents(win);

    if(result) {
        result->readcount = deadWin->stream()->readCount();
        result->writecount = deadWin->stream()->writeCount();
    }

    deadWin->controller()->closeWindow();
}

void glk_window_get_size(winid_t win, glui32* widthptr, glui32* heightptr) {
    auto size = FROM_WINID(win)->size();

    if(widthptr)
        *widthptr = size.width();

    if(heightptr)
        *heightptr = size.height();
}

void glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin) {
    log_trace() << "glk_window_set_arrangement(" << win << ", " << Glk::WindowArrangement::methodString(method) << ", "
                << size << ", " << keywin << ")";

    if(FROM_WINID(win)->windowType() != Glk::Window::Pair) {
        log_warn() << "Cannot set arrangement of window " << win << " of type "
                   << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()) << " (must be of type "
                   << Glk::Window::windowsTypeString(Glk::Window::Pair) << ")";

        return;
    }

    auto pairWindow = static_cast<Glk::PairWindow*>(FROM_WINID(win));

    pairWindow->setArrangement(!keywin ? pairWindow->keyWindow() : FROM_WINID(keywin),
                               Glk::WindowArrangement::fromMethod(method, size));
}

void glk_window_get_arrangement(winid_t win, glui32* methodptr, glui32* sizeptr, winid_t* keywinptr) {
    log_trace() << "glk_window_get_arrangement(" << win << ", " << methodptr << ", " << sizeptr << ", " << keywinptr
                << ")";

    if(FROM_WINID(win)->windowType() != Glk::Window::Pair) {
        log_warn() << "Cannot get arrangement of window " << win << " of type "
                   << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()) << " (must be of type "
                   << Glk::Window::windowsTypeString(Glk::Window::Pair) << ")";

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
    log_trace() << "glk_window_move_cursor(" << win << ", " << xpos << ", " << ypos << ")";

    FROM_WINID(win)->moveCursor(xpos, ypos);
}

void glk_window_set_echo_stream(winid_t win, strid_t str) {
    log_trace() << "glk_window_set_echo_stream(" << win << ", " << str << ")";

    FROM_WINID(win)->stream()->setEchoStream(FROM_STRID(str));
}

strid_t glk_window_get_echo_stream(winid_t win) {
    log_trace() << "glk_window_get_echo_stream(" << win << ")";

    strid_t str = TO_STRID(FROM_WINID(win)->stream()->echoStream());

    log_trace() << "glk_window_get_echo_stream(" << win << ") => " << str;

    return str;
}

winid_t glk_window_iterate(winid_t win, glui32* rockptr) {
    const auto& winList = QGlk::getMainWindow().windowList();

    if(win == NULL) {
        if(winList.empty()) {
            log_trace() << "glk_window_iterate(" << win << ", " << rockptr << ") => " << ((void*) NULL);
            return NULL;
        }

        auto first = winList.first();

        if(rockptr)
            *rockptr = first->rock();

        log_trace() << "glk_window_iterate(" << win << ", " << rockptr << ") => " << TO_WINID(first);

        return TO_WINID(first);
    }

    auto it = winList.cbegin();

    while(it != winList.cend() && (*it++) != FROM_WINID(win));

    if(it == winList.cend()) {
        log_trace() << "glk_window_iterate(" << win << ", " << rockptr << ") => " << ((void*) NULL);
        return NULL;
    }

    if(rockptr)
        *rockptr = (*it)->rock();

    log_trace() << "glk_window_iterate(" << win << ", " << rockptr << ") => " << TO_WINID(*it);

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
    log_trace() << "glk_window_get_sibling(" << win << ")";

    if(!FROM_WINID(win)->parent()) {
        log_warn() << "Cannot return sibling of window " << win << " (root window has no siblings)";

        return NULL;
    }

    Glk::PairWindow* parent = FROM_WINID(win)->parent();
    winid_t sibling = NULL;

    if(parent->keyWindow() == FROM_WINID(win))
        sibling = TO_WINID(parent->secondWindow());
    else if(parent->secondWindow() == FROM_WINID(win))
        sibling = TO_WINID(parent->keyWindow());

    log_trace() << "glk_window_get_sibling(" << win << ") => " << sibling;

    return sibling;
}

winid_t glk_window_get_root() {
    log_trace() << "glk_window_get_root()";

    winid_t rw = TO_WINID(QGlk::getMainWindow().rootWindow());

    log_trace() << "glk_window_get_root() => " << rw;

    return rw;
}

void glk_window_clear(winid_t win) {
    log_trace() << "glk_window_clear(" << win << ")";

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
    log_trace() << "glk_window_set_background_color(" << win << ", " << color << ")";

    FROM_WINID(win)->setBackgroundColor(QColor::fromRgb(color));
}

void glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    log_trace() << "glk_window_fill_rect(" << win << ", " << color << ", "
                << left << ", " << top << ", " << width << ", " << height << ")";

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
    Glk::Blorb::Chunk imgchunk = Glk::Blorb::loadResource(image, Glk::Blorb::ResourceUsage::Picture);

    if(!imgchunk.isValid())
        return FALSE;

    QImage img = QImage::fromData(reinterpret_cast<const uchar*>(imgchunk.data()), imgchunk.length());

    return (FROM_WINID(win)->drawImage(img, val1, val2, img.size()) ? TRUE : FALSE);
}

glui32 glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height) {
    Glk::Blorb::Chunk imgchunk = Glk::Blorb::loadResource(image, Glk::Blorb::ResourceUsage::Picture);

    if(!imgchunk.isValid())
        return FALSE;

    QImage img = QImage::fromData(reinterpret_cast<const uchar*>(imgchunk.data()), imgchunk.length());

    return (FROM_WINID(win)->drawImage(img, val1, val2, QSize(width, height)) ? TRUE : FALSE);
}

void glk_window_flow_break(winid_t win) {
    FROM_WINID(win)->flowBreak();
}
