#include "taskrequest.hpp"

#include <QCoreApplication>

#include "qglk.hpp"

Glk::TaskEvent::TaskEvent(QSemaphore& sem, const std::function<void ()>& tsk) : QEvent(static_cast<QEvent::Type>(Type)), m_Task(tsk), mr_Semaphore(sem), m_Handled(false) {
}

void Glk::TaskEvent::execute() {
    if(m_Handled)
        return;

    m_Task();
    m_Handled = true;

    mr_Semaphore.release(1);
}

void Glk::sendTaskToEventThread(const std::function<void ()>& tsk){
    QSemaphore sem(0);
    TaskEvent* te = new TaskEvent(sem, tsk);
    QCoreApplication::postEvent(&QGlk::getMainWindow(), te, Qt::HighEventPriority);
    sem.acquire(1);
}


