#include "glk.hpp"

#include <QTimer>

#include "qglk.hpp"
#include "event/eventqueue.hpp"
#include "exception.hpp"
#include "thread/taskrequest.hpp"
#include "window/window.hpp"

#include "log/log.hpp"

void glk_tick() {
    if(QGlk::getMainWindow().eventQueue().isInterrupted())
        throw Glk::ExitException(true);

    emit QGlk::getMainWindow().tick();
}

void glk_set_interrupt_handler(void (* func)(void)) {
    QGlk::getMainWindow().setInterruptHandler(func);
}

void glk_exit() {
    QGlk::getMainWindow().close();

    throw Glk::ExitException();
}

void glk_select(event_t* event) {
    log_trace() << "glk_select(" << event << ")";
    log_debug() << "Waiting for event";

    emit QGlk::getMainWindow().poll();
    *event = QGlk::getMainWindow().eventQueue().pop();

    log_debug() << "Received event of type " << to_string(*event);
}

void glk_select_poll(event_t* event) {
    Glk::sendTaskToEventThread([]() {
        QGlk::getMainWindow().synchronize();
    });

    emit QGlk::getMainWindow().poll();

    log_debug() << "Polling events";
    *event = QGlk::getMainWindow().eventQueue().poll();

    if(event->type == evtype_None) {
        log_debug() << "No event received";
    } else {
        log_debug() << "Received event of type " << to_string(*event);
    }
}

void glk_request_char_event(winid_t win) {
    log_trace() << "glk_request_char_event(" << win << ")";

    if(FROM_WINID(win)->controller()->supportsCharInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestCharInput(false);
    else
        log_warn() << FROM_WINID(win) << " does not accept char input.";
}

void glk_request_char_event_uni(winid_t win) {
    log_trace() << "glk_request_char_event_uni(" << win << ")";

    if(FROM_WINID(win)->controller()->supportsCharInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestCharInput(true);
    else
        log_warn() << FROM_WINID(win) << " does not accept char input.";
}

void glk_cancel_char_event(winid_t win) {
    log_trace() << "glk_cancel_char_event(" << win << ")";

    if(FROM_WINID(win)->controller()->supportsCharInput())
        FROM_WINID(win)->controller()->keyboardProvider()->cancelCharInputRequest();
    else
        log_warn() << "Failed to cancel char input event. " << FROM_WINID(win) << " does not accept char input.";
}

void glk_request_line_event(winid_t win, char* buf, glui32 maxlen, glui32 initlen) {
    log_trace() << "glk_request_line_event(" << win << "," << ((void*) buf) << "," << maxlen << "," << initlen << ")";

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestLineInput(buf, maxlen, initlen, false);
    else
        log_warn() << FROM_WINID(win) << " does not accept line input.";
}

void glk_request_line_event_uni(winid_t win, glui32* buf, glui32 maxlen, glui32 initlen) {
    log_trace() << "glk_request_line_event_uni(" << win << "," << ((void*) buf) << "," << maxlen << "," << initlen
                << ")";

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestLineInput(buf, maxlen, initlen, true);
    else
        log_warn() << FROM_WINID(win) << " does not accept line input.";
}

void glk_cancel_line_event(winid_t win, event_t* event) {
    log_trace() << "glk_cancel_line_event(" << win << ")";

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->cancelLineInputRequest(event);
    else
        log_warn() << "Failed to cancel line input event. " << FROM_WINID(win) << " does not accept line input.";
}

void glk_set_echo_line_event(winid_t win, glui32 val) {
    log_trace() << "glk_set_echo_line_event(" << win << ", " << (val != 0) << ")";

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->setLineInputEcho(val != 0);
}

void glk_set_terminators_line_event(winid_t win, glui32* keycodes, glui32 count) {
    log_trace() << "glk_set_terminators_line_event(" << win << ", " << keycodes << ", " << count << ")";

    if(FROM_WINID(win)->controller()->supportsLineInput()) {
        std::set<Qt::Key> keyset;

        if(keycodes) {
            for(size_t ii = 0; ii < count; ++ii) {
                auto qtkeys = Glk::LineInputRequest::toQtKeyTerminators(keycodes[ii]);
                keyset.insert(qtkeys.begin(), qtkeys.end());
            }
        }

        FROM_WINID(win)->controller()->keyboardProvider()->setLineInputTerminators(keyset);
    }
}

void glk_request_mouse_event(winid_t win) {
    log_trace() << "glk_request_mouse_event(" << win << ")";

    if(FROM_WINID(win)->controller()->supportsMouseInput())
        FROM_WINID(win)->controller()->mouseProvider()->requestMouseInput();
    else
        log_warn() << FROM_WINID(win) << " does not accept mouse input.";
}

void glk_cancel_mouse_event(winid_t win) {
    log_trace() << "glk_cancel_mouse_event(" << win << ")";

    if(FROM_WINID(win)->controller()->supportsMouseInput())
        FROM_WINID(win)->controller()->mouseProvider()->cancelMouseInputRequest();
    else
        log_warn() << "Failed to cancel mouse input event. " << FROM_WINID(win) << " does not accept mouse input.";
}

QTimer* s_Timer = NULL;

void glk_request_timer_events(glui32 millisecs) {
    log_trace() << "glk_request_timer_events(" << millisecs << ")";

    Glk::sendTaskToEventThread([&] {
        if(!s_Timer) {
            s_Timer = new QTimer();
            QObject::connect(s_Timer, &QTimer::timeout, &QGlk::getMainWindow().eventQueue(),
                             &Glk::EventQueue::pushTimerEvent);
        }

        if(millisecs == 0)
            s_Timer->stop();
        else
            s_Timer->start(millisecs);
    });
}

void glk_request_hyperlink_event(winid_t win) {
    log_trace() << "glk_request_hyperlink_event(" << win << ")";

//    Glk::sendTaskToEventThread([&] {
//        FROM_WINID(win)->hyperlinkInputProvider()->requestHyperlinkInput();
//    });
}

void glk_cancel_hyperlink_event(winid_t win) {
    log_trace() << "glk_cancel_hyperlink_event(" << win << ")";

//    Glk::sendTaskToEventThread([&] {
//        FROM_WINID(win)->hyperlinkInputProvider()->cancelHyperlinkInputRequest();
//    });
}
