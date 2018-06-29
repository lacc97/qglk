#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "glk.hpp"

#include "thread/taskrequest.hpp"

namespace Glk {
    class EventQueue : public QObject {
        Q_OBJECT
    public:
        EventQueue();
        
        // these two methods should only be called from glk thread
        event_t pop();
        event_t poll();
        
        void interrupt();
        inline bool isInterrupted() const {
            return m_Terminate;
        }
        
    public slots:
        void cleanWindowEvents(winid_t win);
        void push(const event_t& ev);
        void pushTaskEvent(TaskEvent* ev); // push code that should be executed in glk thread
        void pushTimerEvent();
        
    private:
        QQueue<event_t> m_Queue;
        QQueue<TaskEvent*> m_TaskEventQueue;
        QMutex m_AccessMutex;
        QSemaphore m_Semaphore;
        
        bool m_Terminate;
    };
}

#endif
