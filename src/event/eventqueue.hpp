#ifndef EVENTQUEUE_HPP
#define EVENTQUEUE_HPP

#include <QObject>
#include <QQueue>
#include <QMutex>
#include <QSemaphore>

#include "glk.hpp"

#include "thread/taskrequest.hpp"

namespace Glk {
    class Window;

    class EventQueue : public QObject {
        Q_OBJECT
    public:
        explicit EventQueue(QObject* parent = nullptr);
        

        event_t pop();
        event_t popLineEvent(Glk::Window* win);
        event_t poll();
        

        void interrupt();

        [[nodiscard]] inline bool isInterrupted() const {
            return m_Terminate;
        }
        
        [[nodiscard]] inline bool isWaitingOnSemaphore() const {
            return m_Semaphore.available() == 0;
        }

        void requestImmediateSynchronization();
        
    public slots:
        void cleanWindowEvents(winid_t win);
        void push(const event_t& ev);
        void pushTaskEvent(Glk::TaskEvent* ev); // push code that should be executed in glk thread
        void pushTimerEvent();

    signals:
        void canSynchronize();
        
    private:
        QQueue<event_t> m_Queue;
        QQueue<TaskEvent*> m_TaskEventQueue;
        QMutex m_AccessMutex;
        QSemaphore m_Semaphore;
        
        bool m_Terminate;
    };
}

std::ostream& operator<<(std::ostream& os, const event_t& e);

#endif
