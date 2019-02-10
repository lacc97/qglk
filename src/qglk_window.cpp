#include "window/window.hpp"

#include <QDebug>
#include <QSet>

#include "qglk.hpp"

#include "blorb/chunk.hpp"
#include "thread/taskrequest.hpp"
#include "window/blankwindow.hpp"
#include "window/graphicswindow.hpp"
#include "window/pairwindow.hpp"
#include "window/textbufferwindow.hpp"
#include "window/textgridwindow.hpp"

#include "log/log.hpp"

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

// #ifndef NDEBUG
// void printTree(Glk::Window* root) {
//     static int tabs = 0;
//
//     if(!root)
//         return;
//
//     {
//         QDebug debug = qDebug();
//
//         for(int ii = 0; ii < tabs; ii++)
//             debug << "  ";
//
//         debug << root << ": {"
// //               << "parent <=" << root->windowParent() << ";"
// //               << "children <=" << root->children() << ";"
//               << "layout <=" << root->layout() << ";"
//               << "rect <=" << root->rect() << ";";
//
//         if(root->windowType() == Glk::Window::Pair)
//             debug << "key <=" << static_cast<Glk::PairWindow*>(root)->keyWindow() << ";";
//
//         debug << "}";
//     }
//
//     if(root->windowType() == Glk::Window::Pair) {
//         Glk::PairWindow* pw = static_cast<Glk::PairWindow*>(root);
//         tabs++;
//         printTree(pw->firstWindow());
//         printTree(pw->secondWindow());
//         tabs--;
//     }
// }
// #endif

winid_t glk_window_open(winid_t split, glui32 method, glui32 size, glui32 wintype, glui32 rock) {
    Glk::PairWindow* pairw = NULL;
    Glk::Window* neww = NULL;

    if(wintype == Glk::Window::Pair || (!split && s_WindowSet.size() > 0)) {
        trace() << "glk_window_open(" << split << ", "
                << Glk::WindowConstraint::methodString(method).toStdString()
                << ", " << size << ", "
                << Glk::Window::windowsTypeString(wintype).toStdString()
                << ", " << rock << ") => " << ((void*)NULL);

        return NULL;
    }

    Glk::sendTaskToEventThread([&] {
        neww = winFromType(wintype, rock);

        if(split) {
            Glk::PairWindow* prnt = FROM_WINID(split)->windowParent();
            Glk::Window* prntf = NULL;
            Glk::Window* prnts = NULL;

            if(prnt) {
                prntf = prnt->firstWindow();
                prnts = prnt->secondWindow();
            }

            Glk::WindowConstraint* cnt;

            if(Glk::WindowConstraint::isVertical(method)) {
                cnt = new Glk::VerticalWindowConstraint(static_cast<Glk::WindowConstraint::Method>(method), size);
            } else {
                cnt = new Glk::HorizontalWindowConstraint(static_cast<Glk::WindowConstraint::Method>(method), size);
            }

            pairw = new Glk::PairWindow(neww, neww, FROM_WINID(split), cnt);

            if(!prnt) {
                pairw->setWindowParent(NULL);
                QGlk::getMainWindow().setRootWindow(pairw);
            } else {
                prntf = (prntf == FROM_WINID(split) ? pairw : prntf);
                prnts = (prnts == FROM_WINID(split) ? pairw : prnts);

                prnt->constraint()->setupWindows(prnt, prnt->keyWindow(), prntf, prnts);
            }
        } else {
            QGlk::getMainWindow().setRootWindow(neww);
        }
    });

    trace() << "glk_window_open(" << split << ", "
            << Glk::WindowConstraint::methodString(method).toStdString() << ", "
            << size << ", " << Glk::Window::windowsTypeString(wintype).toStdString() << ", "
            << rock << ") => " << TO_WINID(neww);

    return TO_WINID(neww);
}

void glk_window_close(winid_t win, stream_result_t* result) {
    trace() << "glk_window_close(" << win << ", " << result << ")";

    Glk::sendTaskToEventThread([&] {
        Glk::PairWindow* prntw = FROM_WINID(win)->windowParent();

        if(prntw) {
            Glk::Window* prntws = prntw->secondWindow();

            prntw->removeChildWindow(FROM_WINID(win));
            FROM_WINID(win)->orphan();

            if(FROM_WINID(win) == prntws)
                delete prntw;
        } else if(FROM_WINID(win)->parent()) {
            debug() << "Closing root window " << win;

            QGlk::getMainWindow().setRootWindow(NULL);
        }

        QGlk::getMainWindow().eventQueue().cleanWindowEvents(win);

        if(result) {
            result->readcount = FROM_WINID(win)->windowStream()->readCount();
            result->writecount = FROM_WINID(win)->windowStream()->writeCount();
        }

        delete FROM_WINID(win);
    });

// #ifndef NDEBUG
//     printTree(QGlk::getMainWindow().rootWindow());
//     qDebug() << "";
// #endif
}

void glk_window_get_size(winid_t win, glui32* widthptr, glui32* heightptr) {
    QSize s;
    Glk::sendTaskToEventThread([&] {
        s = FROM_WINID(win)->windowSize();
    });

    if(widthptr)
        *widthptr = s.width();

    if(heightptr)
        *heightptr = s.height();
}

void glk_window_set_arrangement(winid_t win, glui32 method, glui32 size, winid_t keywin) {
    trace() << "glk_window_set_arrangement(" << win << ", "
            << Glk::WindowConstraint::methodString(method).toStdString()
            << ", " << size << ", " << keywin << ")";

    if(FROM_WINID(win)->windowType() != Glk::Window::Pair) {
        warn() << "Cannot set arrangement of window " << win << " of type "
               << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()).toStdString()
               << " (must be of type " << Glk::Window::windowsTypeString(Glk::Window::Pair).toStdString() << ")";

        return;
    }

    Glk::sendTaskToEventThread([&] {
        Glk::PairWindow* pw = static_cast<Glk::PairWindow*>(FROM_WINID(win));

        if(keywin) {
            Q_ASSERT_X(pw->isDescendant(FROM_WINID(keywin)), "glk_window_set_arrangement", "keywin must be a descendant of win");
            pw->setKeyWindow(FROM_WINID(keywin)); // assert that keywin is a descendant of win
        }

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

    Glk::sendTaskToEventThread([&] {
        Glk::PairWindow* pw = static_cast<Glk::PairWindow*>(FROM_WINID(win));

        if(methodptr)
            *methodptr = pw->constraint()->method();

        if(sizeptr)
            *sizeptr = pw->constraint()->size();

        if(keywinptr)
            *keywinptr = TO_WINID(pw->keyWindow());
    });
}

void glk_window_move_cursor(winid_t win, glui32 xpos, glui32 ypos) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextGrid) {
        warn() << "Cannot move cursor of window " << win << " of type "
               << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()).toStdString()
               << " (must be of type " << Glk::Window::windowsTypeString(Glk::Window::TextGrid).toStdString() << ")";

        return;
    }

    Glk::sendTaskToEventThread([&] {
        static_cast<Glk::TextGridWindow*>(FROM_WINID(win))->setGridCursor(xpos, ypos);
    });
}

void glk_window_set_echo_stream(winid_t win, strid_t str) {
    trace() << "glk_window_set_echo_stream(" << win << ", " << str << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->windowStream()->setEchoStream(FROM_STRID(str));
    });
}

strid_t glk_window_get_echo_stream(winid_t win) {
    strid_t str;

    Glk::sendTaskToEventThread([&] {
        str = TO_STRID(FROM_WINID(win)->windowStream()->echoStream());
    });

    trace() << "glk_window_get_echo_stream(" << win << ") => " << str;

    return str;
}

QSet<Glk::Window*> s_WindowSet;
winid_t glk_window_iterate(winid_t win, glui32* rockptr) {
    QSet<Glk::Window*>::const_iterator iter;

    if(win == NULL) {
        iter = s_WindowSet.begin();
    } else {
        iter = s_WindowSet.find(FROM_WINID(win));
        iter++;
    }

    if(iter == s_WindowSet.end()) {
        trace() << "glk_window_iterate(" << win << ", " << rockptr << ") => " << ((void*)NULL);
        return NULL;
    }

    if(rockptr)
        *rockptr = (*iter)->rock();

    trace() << "glk_window_iterate(" << win << ", " << rockptr << ") => " << TO_WINID(*iter);

    return TO_WINID(*iter);
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
    Q_ASSERT(win);
    
    if(!FROM_WINID(win)->windowParent()) {
        warn() << "Cannot return sibling of window " << win << " (root window has no siblings)";

        return NULL;
    }

    Glk::PairWindow* prnt = FROM_WINID(win)->windowParent();
    winid_t sibling = NULL;

    if(prnt->keyWindow() == FROM_WINID(win))
        sibling = TO_WINID(prnt->secondWindow());
    else if(prnt->secondWindow() == FROM_WINID(win))
        sibling = TO_WINID(prnt->keyWindow());

    trace() << "glk_window_get_sibling(" << win << ") => " << sibling;
    
    return sibling;
}

winid_t glk_window_get_root() {
    winid_t rw = NULL;

    Glk::sendTaskToEventThread([&] {
        rw = TO_WINID(QGlk::getMainWindow().rootWindow());
    });
    
    trace() << "glk_window_get_root() => " << rw;

    return rw;
}

void glk_window_clear(winid_t win) {
    trace() << "glk_window_clear(" << win << ")";
    
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->clearWindow();
    });
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
    trace() << "glk_window_set_background_color(" << win << ", " << color << ")";
    
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics) {
        warn() << "Cannot change background colour of window " << win << " of type "
               << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()).toStdString()
               << " (must be of type " << Glk::Window::windowsTypeString(Glk::Window::Graphics).toStdString() << ")";
        
        return;
    }

    Glk::sendTaskToEventThread([&] {
        static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->setBackgroundColor(QColor::fromRgb(color));
    });
}

void glk_window_fill_rect(winid_t win, glui32 color, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    trace() << "glk_window_fill_rect(" << win << ", " << color << ", "
            << left << ", " << top << ", " << width << ", " << height << ")";
    
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics) {
        warn() << "Cannot fill rect in window " << win << " of type "
               << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()).toStdString()
               << " (must be of type " << Glk::Window::windowsTypeString(Glk::Window::Graphics).toStdString() << ")";
        
        return;
    }

    Glk::sendTaskToEventThread([&] {
        static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->fillRect(QColor::fromRgb(color), left, top, width, height);
    });
}

void glk_window_erase_rect(winid_t win, glsi32 left, glsi32 top, glui32 width, glui32 height) {
    trace() << "glk_window_erase_rect(" << win << ", " << left << ", "
            << top << ", " << width << ", " << height << ")";
    
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics) {
        warn() << "Cannot erase rect in window " << win << " of type "
               << Glk::Window::windowsTypeString(FROM_WINID(win)->windowType()).toStdString()
               << " (must be of type " << Glk::Window::windowsTypeString(Glk::Window::Graphics).toStdString() << ")";
        
        return;
    }

    Glk::sendTaskToEventThread([&] {
        static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->fillRect(Qt::transparent, left, top, width, height);
    });
}

// style related functions use threads because in the future it should be possible to change
// fonts and colours from QGlk via a menu or something
void glk_stylehint_set(glui32 wintype, glui32 styl, glui32 hint, glsi32 val) {
    if(wintype != Glk::Window::TextBuffer)
        return;

    Glk::sendTaskToEventThread([&] {
        QGlk::getMainWindow().textBufferStyleManager()[static_cast<Glk::Style::Type>(styl)].setHint(hint, val);
    });
}

void glk_stylehint_clear(glui32 wintype, glui32 styl, glui32 hint) {
    if(wintype != Glk::Window::TextBuffer)
        return;

    Glk::sendTaskToEventThread([&] {
        QGlk::getMainWindow().textBufferStyleManager()[static_cast<Glk::Style::Type>(styl)].setHint(hint, QGlk::getMainWindow().defaultStyleManager()[static_cast<Glk::Style::Type>(styl)].getHint(hint));
    });
}

glui32 glk_style_distinguish(winid_t win, glui32 styl1, glui32 styl2) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextBuffer)
        return FALSE;

    bool res;

    Glk::sendTaskToEventThread([&] {
        res = static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl1)] != static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl2)];
    });

    return res;
}

glui32 glk_style_measure(winid_t win, glui32 styl, glui32 hint, glui32* result) {
    if(FROM_WINID(win)->windowType() != Glk::Window::TextBuffer || !result)
        return FALSE;

    bool res;

    Glk::sendTaskToEventThread([&] {
        res = static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->styles()[static_cast<Glk::Style::Type>(styl)].measureHint(hint, result);
    });

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
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics && FROM_WINID(win)->windowType() != Glk::Window::TextBuffer)
        return FALSE;

    Glk::Blorb::Chunk imgchunk = Glk::Blorb::loadResource(image, Glk::Blorb::ResourceUsage::Picture);

    if(!imgchunk.isValid())
        return FALSE;

    QImage img = QImage::fromData(reinterpret_cast<const uchar*>(imgchunk.data()), imgchunk.length()).convertToFormat(QImage::Format_ARGB32_Premultiplied);

    if(img.isNull())
        return FALSE;

    bool res;

    if(FROM_WINID(win)->windowType() == Glk::Window::Graphics) {
        Glk::sendTaskToEventThread([&] {
            res = static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->drawImage(img, val1, val2, img.width(), img.height());
        });
    } else {
        Glk::sendTaskToEventThread([&] {
            res = static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->drawImage(img, val1, img.width(), img.height());
        });
    }

    return res;
}

glui32 glk_image_draw_scaled(winid_t win, glui32 image, glsi32 val1, glsi32 val2, glui32 width, glui32 height) {
    if(FROM_WINID(win)->windowType() != Glk::Window::Graphics && FROM_WINID(win)->windowType() != Glk::Window::TextBuffer)
        return FALSE;

    Glk::Blorb::Chunk imgchunk = Glk::Blorb::loadResource(image, Glk::Blorb::ResourceUsage::Picture);

    if(!imgchunk.isValid())
        return FALSE;

    QImage img = QImage::fromData(reinterpret_cast<const uchar*>(imgchunk.data()), imgchunk.length()).convertToFormat(QImage::Format_ARGB32_Premultiplied);;

    if(img.isNull())
        return FALSE;

    bool res;

    if(FROM_WINID(win)->windowType() == Glk::Window::Graphics) {
        Glk::sendTaskToEventThread([&] {
            res = static_cast<Glk::GraphicsWindow*>(FROM_WINID(win))->drawImage(img, val1, val2, width, height);
        });
    } else {
        Glk::sendTaskToEventThread([&] {
            res = static_cast<Glk::TextBufferWindow*>(FROM_WINID(win))->drawImage(img, val1, width, height);
        });
    }

    return res;
}

void glk_window_flow_break(winid_t win) {
    // TODO implement
}
