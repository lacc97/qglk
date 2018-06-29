#ifndef TASKREQUEST_HPP
#define TASKREQUEST_HPP

#include <functional>

#include <QEvent>
#include <QSemaphore>

namespace Glk {
    class TaskEvent : public QEvent {
        public:
            static const int Type = QEvent::User + 1;

            TaskEvent(QSemaphore& sem, const std::function<void(void)>& tsk);

            void execute();

        private:
            std::function<void(void)> m_Task;
            QSemaphore& mr_Semaphore;
            bool m_Handled;
    };
    
    void sendTaskToEventThread(const std::function<void(void)>& tsk);
    void sendTaskToGlkThread(const std::function<void(void)>& tsk);
}

#endif
