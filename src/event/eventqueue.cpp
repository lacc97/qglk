#include "eventqueue.hpp"

#include <QMutexLocker>

Glk::EventQueue::EventQueue() : m_Queue(), m_Semaphore(0) {
}

event_t Glk::EventQueue::pop() {
    m_Semaphore.acquire(1);

    QMutexLocker ml(&m_AccessMutex);

    return m_Queue.dequeue();
}

event_t Glk::EventQueue::poll() {
    QMutexLocker ml(&m_AccessMutex);

    if(m_Queue.isEmpty())
        return event_t {evtype_None, NULL, 0, 0};

    for(int ii = 0; ii < m_Queue.size(); ii++) {
        switch(m_Queue[ii].type) {
            case evtype_Timer:
            case evtype_Arrange:
            case evtype_SoundNotify:
                event_t ev = m_Queue.takeAt(ii);
                m_Semaphore.acquire(1);
                return ev;
        }
    }

    return event_t {evtype_None, NULL, 0, 0};
}

void Glk::EventQueue::push(const event_t& ev) {
    QMutexLocker ml(&m_AccessMutex);

    m_Queue.enqueue(ev);

    m_Semaphore.release(1);
}

void Glk::EventQueue::pushTimerEvent() {
    push(event_t {evtype_Timer, NULL, 0, 0});
}
