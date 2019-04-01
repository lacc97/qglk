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

void glk_set_interrupt_handler(void (*func)(void)) {
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

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestCharInput(false);
    });
}

void glk_request_char_event_uni(winid_t win) {
    log_trace() << "glk_request_char_event_uni(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestCharInput(true);
    });
}

void glk_cancel_char_event(winid_t win) {
    log_trace() << "glk_cancel_char_event(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->cancelCharInputRequest();
    });
}

void glk_request_line_event(winid_t win, char* buf, glui32 maxlen, glui32 initlen) {
    log_trace() << "glk_request_line_event(" << win << "," << ((void*)buf) << "," << maxlen << "," << initlen << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestLineInput(buf, maxlen, initlen, false);
    });
}

void glk_request_line_event_uni(winid_t win, glui32* buf, glui32 maxlen, glui32 initlen) {
    log_trace() << "glk_request_line_event_uni(" << win << "," << ((void*)buf) << "," << maxlen << "," << initlen << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestLineInput(buf, maxlen, initlen, true);
    });
}

void glk_cancel_line_event(winid_t win, event_t* event) {
    log_trace() << "glk_cancel_line_event(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->cancelLineInputRequest(event);
    });
}

void glk_set_echo_line_event(winid_t win, glui32 val) {
    log_trace() << "glk_set_echo_line_event(" << win << ", " << (val != 0) << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->setLineEcho(val != 0);
    });
}

void glk_set_terminators_line_event(winid_t win, glui32* keycodes, glui32 count) {
    log_trace() << "glk_set_terminators_line_event(" << win << ", " << keycodes << ", " << count << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->setTerminators(keycodes, count);
    });
}

void glk_request_mouse_event(winid_t win) {
    log_trace() << "glk_request_mouse_event(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->mouseInputProvider()->requestMouseInput();
    });
}

void glk_cancel_mouse_event(winid_t win) {
    log_trace() << "glk_cancel_mouse_event(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->mouseInputProvider()->cancelMouseInputRequest();
    });
}

QTimer* s_Timer = NULL;
void glk_request_timer_events(glui32 millisecs) {
    log_trace() << "glk_request_timer_events(" << millisecs << ")";

    Glk::sendTaskToEventThread([&] {
        if(!s_Timer) {
            s_Timer = new QTimer();
            QObject::connect(s_Timer, &QTimer::timeout, &QGlk::getMainWindow().eventQueue(), &Glk::EventQueue::pushTimerEvent);
        }

        if(millisecs == 0)
            s_Timer->stop();
        else
            s_Timer->start(millisecs);
    });
}

void glk_request_hyperlink_event(winid_t win) {
    log_trace() << "glk_request_hyperlink_event(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->hyperlinkInputProvider()->requestHyperlinkInput();
    });
}

void glk_cancel_hyperlink_event(winid_t win) {
    log_trace() << "glk_cancel_hyperlink_event(" << win << ")";

    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->hyperlinkInputProvider()->cancelHyperlinkInputRequest();
    });
}
