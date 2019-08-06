#include "taskrequest.hpp"

#include <QCoreApplication>
#include <QThread>

#include "qglk.hpp"

Glk::TaskEvent::TaskEvent(const std::function<void ()>& tsk) : QEvent(static_cast<QEvent::Type>(Type)), m_Task(tsk), m_Handled(false) {
}

void Glk::TaskEvent::execute() {
    if(m_Handled)
        return;

    m_Task();
    m_Handled = true;
}

Glk::SynchronizedTaskEvent::SynchronizedTaskEvent(QSemaphore& sem, const std::function<void ()>& tsk) : TaskEvent(tsk), mr_Semaphore(sem) {
}

void Glk::SynchronizedTaskEvent::execute() {
    TaskEvent::execute();

    mr_Semaphore.release(1);
}

bool Glk::onEventThread() {
    return QGlk::getMainWindow().thread() == QThread::currentThread();
}

bool Glk::onGlkThread() {
    return QGlk::getMainWindow().glkRunnable()->glkThread() == QThread::currentThread();
}

void Glk::postTaskToEventThread(const std::function<void(void)>& tsk) {
    if(!onEventThread())
        QCoreApplication::postEvent(&QGlk::getMainWindow(), new TaskEvent(tsk), Qt::HighEventPriority);
    else
        tsk();
}

void Glk::postTaskToGlkThread(const std::function<void ()>& tsk) {
    if(!onGlkThread())
        QGlk::getMainWindow().eventQueue().pushTaskEvent(new TaskEvent(tsk));
    else
        tsk();
}

// This function should only be called from the glk thread.
void Glk::sendTaskToEventThread(const std::function<void ()>& tsk) {
    if(!onEventThread()) {
        QSemaphore sem(0);
        TaskEvent* te = new SynchronizedTaskEvent(sem, tsk);
        QCoreApplication::postEvent(&QGlk::getMainWindow(), te, Qt::HighEventPriority);
        sem.acquire(1);
    } else {
        tsk();
    }
}

