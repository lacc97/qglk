#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "glk.hpp"

namespace Glk {
    class EventQueue : public QObject {
        Q_OBJECT
    public:
        EventQueue();
        
        event_t pop();
        event_t poll();
        
        void interrupt();
        inline bool isInterrupted() const {
            return m_Terminate;
        }
        
    public slots:
        void push(const event_t& ev);
        void pushTimerEvent();
        
    private:
        QQueue<event_t> m_Queue;
        QMutex m_AccessMutex;
        QSemaphore m_Semaphore;
        
        bool m_Terminate;
    };
}

#endif
