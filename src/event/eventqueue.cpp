#include "eventqueue.hpp"

#include "exception.hpp"

#include <QMutexLocker>

// #define evtype_TaskEvent (0xfafbfcfd)
#define evtype_TaskEvent (evtype_None)

Glk::EventQueue::EventQueue(QObject* parent) : QObject(parent), m_Queue(), m_Semaphore(0), m_Terminate(false) {
}

event_t Glk::EventQueue::pop() {
    event_t ev;

    do {
        m_Semaphore.acquire(1);

        QMutexLocker ml(&m_AccessMutex);

        if(m_Terminate)
            throw Glk::ExitException(true);

        ev = m_Queue.dequeue();
        
        if(ev.type == evtype_TaskEvent) {
            TaskEvent* tev = m_TaskEventQueue.dequeue();
            tev->execute();
            delete tev;
        }
    } while(ev.type == evtype_TaskEvent);
    
    return ev;
}

event_t Glk::EventQueue::poll() {
    QMutexLocker ml(&m_AccessMutex);

    if(m_Terminate)
        throw Glk::ExitException(true);

    if(m_Queue.isEmpty())
        return event_t {evtype_None, NULL, 0, 0};

    for(int ii = 0; ii < m_Queue.size(); ii++) {
        event_t ev;

        switch(m_Queue[ii].type) {
            case evtype_TaskEvent: {
                ev = m_Queue.takeAt(ii--);
                TaskEvent* tev = m_TaskEventQueue.dequeue();
                tev->execute();
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

    return event_t {evtype_None, NULL, 0, 0};
}

void Glk::EventQueue::interrupt() {
    m_Terminate = true;
    m_Semaphore.release(1);
}

void Glk::EventQueue::cleanWindowEvents(winid_t win) {
    QMutexLocker ml(&m_AccessMutex);

    for(int ii = 0; ii < m_Queue.size(); ii++) {
        if(m_Queue[ii].win == win)
            m_Queue.removeAt(ii--);
    }
}

void Glk::EventQueue::push(const event_t& ev) {
    QMutexLocker ml(&m_AccessMutex);

    m_Queue.enqueue(ev);

    m_Semaphore.release(1);
}

void Glk::EventQueue::pushTaskEvent(Glk::TaskEvent* ev) {
    Q_ASSERT(ev);
    
    QMutexLocker ml(&m_AccessMutex);

    m_Queue.enqueue(event_t {evtype_TaskEvent, NULL, 0, 0});
    m_TaskEventQueue.enqueue(ev);

    m_Semaphore.release(1);
}

void Glk::EventQueue::pushTimerEvent() {
    event_t ev {evtype_Timer, NULL, 0, 0};

    QMutexLocker ml(&m_AccessMutex);

    for(int ii = 0; ii < m_Queue.size(); ii++) {
        switch(m_Queue[ii].type) {
            case evtype_Timer:
                m_Queue.removeAt(ii);
                m_Queue.enqueue(ev);
                return;
        }
    }

    m_Queue.enqueue(ev);

    m_Semaphore.release(1);
}
