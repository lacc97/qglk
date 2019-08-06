#ifndef TASKREQUEST_HPP
#define TASKREQUEST_HPP

#include <functional>

#include <QEvent>
#include <QSemaphore>

namespace Glk {
    class TaskEvent : public QEvent {
        Q_DISABLE_COPY(TaskEvent)
        public:
            static const int Type = QEvent::User + 1;

            TaskEvent(const std::function<void(void)>& tsk);

            virtual void execute();
            
            inline bool handled() const {
                return m_Handled;
            }

        private:
            std::function<void(void)> m_Task;
            bool m_Handled;
    };
    
    class SynchronizedTaskEvent : public TaskEvent {
    public:
        SynchronizedTaskEvent(QSemaphore& sem, const std::function<void(void)>& tsk);
        
        virtual void execute() override;
        
    private:
        QSemaphore& mr_Semaphore;
    };

    [[nodiscard]] bool onEventThread();
    [[nodiscard]] bool onGlkThread();


    void postTaskToEventThread(const std::function<void(void)>& tsk);

    void postTaskToGlkThread(const std::function<void(void)>& tsk);
    
    void sendTaskToEventThread(const std::function<void(void)>& tsk);
}

#endif
