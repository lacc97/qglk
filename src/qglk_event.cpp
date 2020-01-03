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
    SPDLOG_TRACE("glk_select({})", (void*)event);
    SPDLOG_DEBUG("Waiting for event...");

    emit QGlk::getMainWindow().poll();
    *event = QGlk::getMainWindow().eventQueue().pop();

    SPDLOG_DEBUG("Received event of type {}", *event);
}

void glk_select_poll(event_t* event) {
    Glk::sendTaskToEventThread([]() {
        QGlk::getMainWindow().synchronize();
    });

    emit QGlk::getMainWindow().poll();

    SPDLOG_DEBUG("Polling events");
    *event = QGlk::getMainWindow().eventQueue().poll();

    if(event->type == evtype_None) {
        SPDLOG_DEBUG("No event received");
    } else {
        SPDLOG_DEBUG("Received event of type {}", *event);
    }
}

void glk_request_char_event(winid_t win) {
    SPDLOG_TRACE("glk_request_char_event({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsCharInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestCharInput(false);
    else
        spdlog::warn("{} does not accept char input", wrap::ptr(win));
}

void glk_request_char_event_uni(winid_t win) {
    SPDLOG_TRACE("glk_request_char_event_uni({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsCharInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestCharInput(true);
    else
        spdlog::warn("{} does not accept char input.", wrap::ptr(win));
}

void glk_cancel_char_event(winid_t win) {
    SPDLOG_TRACE("glk_cancel_char_event({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsCharInput())
        FROM_WINID(win)->controller()->keyboardProvider()->cancelCharInputRequest();
    else
        spdlog::warn("Failed to cancel char input event. {} does not accept char input.", wrap::ptr(win));
}

void glk_request_line_event(winid_t win, char* buf, glui32 maxlen, glui32 initlen) {
    SPDLOG_TRACE("glk_request_line_event({}, {}, {}, {})", wrap::ptr(win), (void*)buf, maxlen, initlen);

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestLineInput(buf, maxlen, initlen, false);
    else
        spdlog::warn("{} does not accept line input.", wrap::ptr(win));
}

void glk_request_line_event_uni(winid_t win, glui32* buf, glui32 maxlen, glui32 initlen) {
    SPDLOG_TRACE("glk_request_line_event_uni({}, {}, {}, {})", wrap::ptr(win), (void*)buf, maxlen, initlen);

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->requestLineInput(buf, maxlen, initlen, true);
    else
        spdlog::warn("{} does not accept line input.", wrap::ptr(win));
}

void glk_cancel_line_event(winid_t win, event_t* event) {
    SPDLOG_TRACE("glk_cancel_line_event({}, {})", wrap::ptr(win), wrap::ptr(event));

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->cancelLineInputRequest(event);
    else
        spdlog::warn("Failed to cancel line input event. {} does not accept line input.", wrap::ptr(win));
}

void glk_set_echo_line_event(winid_t win, glui32 val) {
    SPDLOG_TRACE("glk_set_echo_line_event({}, {})", wrap::ptr(win), (val != 0));

    if(FROM_WINID(win)->controller()->supportsLineInput())
        FROM_WINID(win)->controller()->keyboardProvider()->setLineInputEcho(val != 0);
}

void glk_set_terminators_line_event(winid_t win, glui32* keycodes, glui32 count) {
    SPDLOG_TRACE("glk_set_terminators_line_event({}, {}, {})", wrap::ptr(win), (void*)keycodes, count);

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
    SPDLOG_TRACE("glk_request_mouse_event({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsMouseInput())
        FROM_WINID(win)->controller()->mouseProvider()->requestMouseInput();
    else
        spdlog::warn("{} does not accept mouse input.", wrap::ptr(win));
}

void glk_cancel_mouse_event(winid_t win) {
    SPDLOG_TRACE("glk_cancel_mouse_event({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsMouseInput())
        FROM_WINID(win)->controller()->mouseProvider()->cancelMouseInputRequest();
    else
        spdlog::warn("Failed to cancel mouse input event. {} does not accept mouse input.", wrap::ptr(win));
}

QTimer* s_Timer = NULL;

void glk_request_timer_events(glui32 millisecs) {
    SPDLOG_TRACE("glk_request_timer_events({})", millisecs);

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
    SPDLOG_TRACE("glk_request_hyperlink_event({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsHyperlinkInput())
        FROM_WINID(win)->controller()->hyperlinkProvider()->requestHyperlinkInput();
    else
        spdlog::warn("{} does not accept hyperlinks", wrap::ptr(win));
}

void glk_cancel_hyperlink_event(winid_t win) {
    SPDLOG_TRACE("glk_cancel_hyperlink_event({})", wrap::ptr(win));

    if(FROM_WINID(win)->controller()->supportsHyperlinkInput())
        FROM_WINID(win)->controller()->hyperlinkProvider()->cancelHyperlinkInputRequest();
    else
        spdlog::warn("Failed to cancel hyperlink request. {} does not accept hyperlinks.", wrap::ptr(win));
}
