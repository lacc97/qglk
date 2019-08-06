#include "eventqueue.hpp"

#include "exception.hpp"

#include <QMutexLocker>

// #define evtype_TaskEvent (0xfafbfcfd)
#define evtype_TaskEvent (0xfafbfcfd)

Glk::EventQueue::EventQueue(QObject* parent)
    : QObject{parent},
      m_Queue{},
      m_Semaphore{0},
      m_Terminate{false} {
}

void Glk::EventQueue::requestImmediateSynchronization() {
    QMutexLocker ml(&m_AccessMutex);

    if(m_Queue.empty()) {
        m_Queue.push_front({evtype_None});
        m_Semaphore.release(1);
    }
}

event_t Glk::EventQueue::pop() {
    event_t ev;

    do {
        emit canSynchronize();

        m_Semaphore.acquire(1);

        QMutexLocker ml(&m_AccessMutex);

        if(m_Terminate)
            throw Glk::ExitException(true);

        ev = m_Queue.dequeue();

        if(ev.type == evtype_TaskEvent) {
            TaskEvent* tev = m_TaskEventQueue.dequeue();
            ml.unlock();
            tev->execute();
            ml.relock();
            delete tev;
        }
    } while(ev.type == evtype_TaskEvent || ev.type == evtype_None);

    emit canSynchronize();

    return ev;
}

event_t Glk::EventQueue::poll() {
    emit canSynchronize();

    QMutexLocker ml(&m_AccessMutex);

    if(m_Terminate)
        throw Glk::ExitException(true);

    if(m_Queue.isEmpty())
        return event_t{evtype_None, NULL, 0, 0};

    for(int ii = 0; ii < m_Queue.size(); ii++) {
        event_t ev;

        switch(m_Queue[ii].type) {
            case evtype_TaskEvent: {
                ev = m_Queue.takeAt(ii--);
                TaskEvent* tev = m_TaskEventQueue.dequeue();
                ml.unlock();
                tev->execute();     // TODO this bit looks dodgy
                ml.relock();
                delete tev;
                m_Semaphore.acquire(1);
                break;
            }

            case evtype_Timer:
            case evtype_Arrange:
            case evtype_SoundNotify: {
                ev = m_Queue.takeAt(ii);
                m_Semaphore.acquire(1);
                return ev;
            }
        }
    }

    return event_t{evtype_None, NULL, 0, 0};
}

void Glk::EventQueue::interrupt() {
    m_Terminate = true;
    m_Semaphore.release(1);
}

void Glk::EventQueue::cleanWindowEvents(winid_t win) {
    QMutexLocker ml(&m_AccessMutex);

    for(int ii = 0; ii < m_Queue.size(); ii++) {
        if(m_Queue[ii].win == win) {
            m_Queue.removeAt(ii--);
            m_Semaphore.acquire(1);
        }
    }
}

void Glk::EventQueue::push(const event_t& ev) {
    QMutexLocker ml(&m_AccessMutex);

    switch(ev.type) {
        case evtype_Arrange:
        case evtype_Redraw:
        case evtype_Timer:
            for(int ii = 0; ii < m_Queue.size(); ii++) {
                if(m_Queue[ii].type == ev.type && (ev.type == evtype_Timer || m_Queue[ii].win == ev.win)) {
                    m_Queue.removeAt(ii);
                    m_Queue.enqueue(ev);
                    return;
                }
            }
    }

    m_Queue.enqueue(ev);
    m_Semaphore.release(1);
}

void Glk::EventQueue::pushTaskEvent(Glk::TaskEvent* ev) {
    Q_ASSERT(ev);

    QMutexLocker ml(&m_AccessMutex);

    m_Queue.enqueue(event_t{evtype_TaskEvent, NULL, 0, 0});
    m_TaskEventQueue.enqueue(ev);

    m_Semaphore.release(1);
}

void Glk::EventQueue::pushTimerEvent() {
    push({evtype_Timer, NULL, 0, 0});
}

const std::string eventTypeName(glui32 t) {
    switch(t) {
        case evtype_None:
            return "None";

        case evtype_Timer:
            return "Timer";

        case evtype_CharInput:
            return "CharInput";

        case evtype_LineInput:
            return "LineInput";

        case evtype_MouseInput:
            return "MouseInput";

        case evtype_Arrange:
            return "Arrange";

        case evtype_Redraw:
            return "Redraw";

        case evtype_SoundNotify:
            return "SoundNotify";

        case evtype_Hyperlink:
            return "HyperLink";

        case evtype_VolumeNotify:
            return "VolumeNotify";

        default:
            return "Unknown";
    }
}

std::ostream& operator<<(std::ostream& os, const event_t& e) {
    return os << eventTypeName(e.type) << "{" << e.win << ", " << e.val1 << ", " << e.val2 << "}";
}
