#include "glk.hpp"

#include <QTimer>

#include "qglk.hpp"
#include "event/eventqueue.hpp"
#include "exception.hpp"
#include "thread/taskrequest.hpp"
#include "window/window.hpp"

void glk_tick() {
    if(QGlk::getMainWindow().eventQueue().isInterrupted())
        throw Glk::ExitException(true);
}

void glk_set_interrupt_handler(void (*func)(void)) {
    QGlk::getMainWindow().setInterruptHandler(func);
}

void glk_exit() {
    QGlk::getMainWindow().close();

    throw Glk::ExitException();
}

void glk_select(event_t* event) {
    *event = QGlk::getMainWindow().eventQueue().pop();
}

void glk_select_poll(event_t* event) {
    QGlk::getMainWindow().update();
    *event = QGlk::getMainWindow().eventQueue().poll();
}

void glk_request_char_event(winid_t win) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestCharInput(false);
    });
}

void glk_request_char_event_uni(winid_t win) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestCharInput(true);
    });
}

void glk_cancel_char_event(winid_t win) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->cancelCharInputRequest();
    });
}

void glk_request_line_event(winid_t win, char* buf, glui32 maxlen, glui32 initlen) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestLineInput(buf, maxlen, initlen, false);
    });
}

void glk_request_line_event_uni(winid_t win, glui32* buf, glui32 maxlen, glui32 initlen) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->requestLineInput(buf, maxlen, initlen, true);
    });
}

void glk_cancel_line_event(winid_t win, event_t* event) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->cancelLineInputRequest(event);
    });
}

void glk_set_echo_line_event(winid_t win, glui32 val) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->setLineEcho(val != 0);
    });
}

void glk_set_terminators_line_event(winid_t win, glui32* keycodes, glui32 count) {
    Glk::sendTaskToEventThread([&] {
        FROM_WINID(win)->keyboardInputProvider()->setTerminators(keycodes, count);
    });
}

void glk_request_mouse_event(winid_t win) {
    Glk::sendTaskToEventThread([&]{
        FROM_WINID(win)->mouseInputProvider()->requestMouseInput();
    });
}

void glk_cancel_mouse_event(winid_t win) {
    Glk::sendTaskToEventThread([&]{
        FROM_WINID(win)->mouseInputProvider()->cancelMouseInputRequest();
    });
}

QTimer* s_Timer = NULL;
void glk_request_timer_events(glui32 millisecs) {
    Glk::sendTaskToEventThread([&] {
        if(!s_Timer) {
            s_Timer = new QTimer();
            QObject::connect(s_Timer, SIGNAL(timeout()), &QGlk::getMainWindow().eventQueue(), SLOT(pushTimerEvent()));
        }

        if(millisecs == 0)
            s_Timer->stop();

        s_Timer->start(millisecs);
    });
}

void glk_request_hyperlink_event(winid_t win) {
    
}

void glk_cancel_hyperlink_event(winid_t win) {
    
}
