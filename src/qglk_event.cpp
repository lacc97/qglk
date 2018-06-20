#include "glk.hpp"

#include <QTimer>

#include "qglk.hpp"
#include "event/eventqueue.hpp"
#include "thread/taskrequest.hpp"
#include "window/window.hpp"

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
