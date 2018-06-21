#include "window/window.hpp"

#include <QSet>

#include "qglk.hpp"

#include "thread/taskrequest.hpp"
#include "window/blankwindow.hpp"
#include "window/graphicswindow.hpp"
#include "window/pairwindow.hpp"
#include "window/textbufferwindow.hpp"
#include "window/textgridwindow.hpp"

// Glk::Window* s_Root = NULL;

namespace {
    Glk::Window* winFromType(glui32 wintype, glui32 rock) {
        switch(wintype) {
            case Glk::Window::Graphics:
                return new Glk::GraphicsWindow(rock);

            case Glk::Window::TextBuffer:
                return new Glk::TextBufferWindow(rock);

            case Glk::Window::TextGrid:
                return new Glk::TextGridWindow(rock);

            default:
                return new Glk::BlankWindow(rock);
        }
    }
}

winid_t glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock) {
    Glk::PairWindow* pairw = NULL;
    Glk::Window* neww;

    if(split && FROM_WINID(split)->windowType() == Glk::Window::Pair)
        return NULL;

    Glk::sendTaskToEventThread([&] {
        neww = winFromType(wintype, rock);

        if(split) {
            Glk::PairWindow* prnt = FROM_WINID(split)->windowParent();

            Glk::WindowConstraint* cnt;

            if(bool(method & Glk::WindowConstraint::Above) || bool(method & Glk::WindowConstraint::Below)) {
                cnt = new Glk::VerticalWindowConstraint(static_cast<Glk::WindowConstraint::Method>(method), size);
            } else {
                cnt = new Glk::HorizontalWindowConstraint(static_cast<Glk::WindowConstraint::Method>(method), size);
            }

            pairw = new Glk::PairWindow(neww, FROM_WINID(split), cnt);

            if(!prnt) {
                QGlk::getMainWindow().setCentralWidget(pairw);
                pairw->setWindowParent(NULL);
            } else {
                Glk::Window* prntk = (prnt->keyWindow() == FROM_WINID(split) ? pairw : prnt->keyWindow());
                Glk::Window* prnts = (prnt->splitWindow() == FROM_WINID(split) ? pairw : prnt->splitWindow());

                prnt->constraint()->setupWindows(prnt, prntk, prnts);
            }
        } else {
            QGlk::getMainWindow().setCentralWidget(neww);
            neww->setWindowParent(NULL);
        }
    });

    return TO_WINID(neww);
}

void glk_window_close(winid_t win, stream_result_t* result) {
    Glk::PairWindow* prntw = static_cast<Glk::PairWindow*>(FROM_WINID(win)->windowParent());

    if(!prntw) {
        if(FROM_WINID(win)->parent()) {
            Glk::sendTaskToEventThread([&] {
                FROM_WINID(win)->unparent();
            });
        }
    } else {
        if(FROM_WINID(win) == prntw->splitWindow()) {
            Glk::sendTaskToEventThread([&] {
                prntw->removeChildWindow(FROM_WINID(win));
            });
            glk_window_close(TO_WINID(prntw), NULL);
        } else {
            Glk::sendTaskToEventThread([&] {
                prntw->removeChildWindow(FROM_WINID(win));
            });
        }
    }

    // TODO remove events associated with win

    if(result) {
        result->readcount = FROM_STRID(FROM_WINID(win)->windowStream())->readCount();
        result->writecount = FROM_STRID(FROM_WINID(win)->windowStream())->writeCount();
    }

    Glk::sendTaskToEventThread([&] {
        delete FROM_WINID(win);
    });
}

void glk_window_get_size(winid_t win, glui32* widthptr, glui32* heightptr) {
    QSize s = FROM_WINID(win)->windowSize();

    if(widthptr)
        *widthptr = s.width();

    if(heightptr)
        *heightptr = s.height();
}

void glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin) {
    if(FROM_WINID(win)->windowType() != Glk::Window::Pair)
        return;

    Glk::PairWindow* pw = static_cast<Glk::PairWindow*>(FROM_WINID(win));

    if(pw->splitWindow() != FROM_WINID(keywin) && pw->keyWindow() != FROM_WINID(keywin))
        return;

    if(pw->splitWindow() == FROM_WINID(keywin))
        pw->swapWindows();

    Glk::sendTaskToEventThread([&] {
        if(bool(method & Glk::WindowConstraint::Above) || bool(method & Glk::WindowConstraint::Below)) {
            pw->setConstraint(new Glk::VerticalWindowConstraint(static_cast<Glk::WindowConstraint::Method>(method), size));
        } else {
            pw->setConstraint(new Glk::HorizontalWindowConstraint(static_cast<Glk::WindowConstraint::Method>(method), size));
        }
    });
}

void glk_window_get_arrangement(winid_t win, glui32* methodptr, glui32* sizeptr, winid_t* keywinptr) {
    if(FROM_WINID(win)->windowType() != Glk::Window::Pair)
        return;

    Glk::PairWindow* pw = static_cast<Glk::PairWindow*>(FROM_WINID(win));

    if(methodptr)
        *methodptr = pw->constraint()->method();

    if(sizeptr)
        *sizeptr = pw->constraint()->size();

    if(keywinptr)
        *keywinptr = TO_WINID(pw->keyWindow());
}

void glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextGrid)
        return;

    static_cast<Glk::TextGridWindow*>(FROM_WINID(win))->setGridCursor(xpos, ypos);
}

void glk_window_set_echo_stream(winid_t win, strid_t str) {
    FROM_WINID(win)->windowStream()->setEchoStream(FROM_STRID(str));
}

strid_t glk_window_get_echo_stream(winid_t win) {
    return TO_STRID(FROM_WINID(win)->windowStream()->echoStream());
}

QSet<Glk::Window*> s_WindowSet;
winid_t glk_window_iterate(winid_t win, glui32* rockptr) {
    if(win == NULL) {
        auto iter = s_WindowSet.begin();

        if(rockptr)
            *rockptr = (*iter)->rock();

        return TO_WINID(*iter);
    }

    auto iter = s_WindowSet.find(FROM_WINID(win));
    iter++;

    if(iter != s_WindowSet.end()) {
        if(rockptr)
            *rockptr = (*iter)->rock();

        return TO_WINID(*iter);
    } else
        return NULL;
}

glui32 glk_window_get_rock(winid_t win) {
    return FROM_WINID(win)->rock();
}

glui32 glk_window_get_type(winid_t win) {
    return FROM_WINID(win)->windowType();
}

winid_t glk_window_get_parent(winid_t win) {
    return TO_WINID(FROM_WINID(win)->windowParent());
}

winid_t glk_window_get_sibling(winid_t win) {
    if(!FROM_WINID(win)->windowParent())
        return NULL;

    Glk::PairWindow* prnt = FROM_WINID(win)->windowParent();

    if(prnt->keyWindow() == FROM_WINID(win))
        return TO_WINID(prnt->splitWindow());
    else if(prnt->splitWindow() == FROM_WINID(win))
        return TO_WINID(prnt->keyWindow());

    return NULL;
}

winid_t glk_window_get_root() {
    return TO_WINID(QGlk::getMainWindow().rootWindow());
}

void glk_window_clear(winid_t win) {
    FROM_WINID(win)->clearWindow();
}

strid_t glk_window_get_stream(winid_t win) {
    return TO_STRID(FROM_WINID(win)->windowStream());
}

void glk_set_window(winid_t win) {
    if(!win)
        glk_stream_set_current(NULL);
    else
        glk_stream_set_current(TO_STRID(FROM_WINID(win)->windowStream()));
}

void glk_window_set_background_color(winid_t win, glui32 color) {
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics)
        return;

    static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->setBackgroundColor(QColor::fromRgba(color));
}

void glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics)
        return;

    static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->fillRect(QColor::fromRgb(color), left, top, width, height);
}

void glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics)
        return;

    static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->fillRect(Qt::transparent, left, top, width, height);
}

void glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val) {
    if(wintype != Glk::Window::TextBuffer)
        return;

    QGlk::getMainWindow().textBufferStyleManager()[static_cast<Glk::Style::Type>(styl)].setHint(hint, val);
}

void glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint) {
    if(wintype != Glk::Window::TextBuffer)
        return;

    QGlk::getMainWindow().textBufferStyleManager()[static_cast<Glk::Style::Type>(styl)].setHint(hint, QGlk::getMainWindow().defaultStyleManager()[static_cast<Glk::Style::Type>(styl)].getHint(hint));
}

glui32 glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextBuffer)
        return FALSE;

    return static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl1)] != static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl2)];
}

glui32 glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32* result) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextBuffer || !result)
        return FALSE;

    return static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl)].measureHint(hint, result);
}
