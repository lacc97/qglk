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

void Glk::postTaskToGlkThread(const std::function<void ()>& tsk) {
    if(QGlk::getMainWindow().glkRunnable()->glkThread() != QThread::currentThread())
        QGlk::getMainWindow().eventQueue().pushTaskEvent(new TaskEvent(tsk));
    else
        tsk();
}

// This function should only be called from the glk thread.
void Glk::sendTaskToEventThread(const std::function<void ()>& tsk) {
    if(QGlk::getMainWindow().thread() != QThread::currentThread()) {
        QSemaphore sem(0);
        TaskEvent* te = new SynchronizedTaskEvent(sem, tsk);
        QCoreApplication::sendEvent(&QGlk::getMainWindow(), te);
        sem.acquire(1);
    } else {
        tsk();
    }
}

