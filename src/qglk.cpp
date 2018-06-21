#include "qglk.hpp"
#include "ui_qglk.h"

#include <QResizeEvent>
#include <QThread>
#include <QThreadPool>

#include "glk.hpp"

#include "exception.hpp"

void Glk::Runnable::run() {
    try {
        glk_main();
    } catch(Glk::ExitException& ex) {
        if(ex.interrupted() && bool(QGlk::getMainWindow().interruptHandler()))
            QGlk::getMainWindow().interruptHandler()();
    }

    return;
}

QGlk::QGlk(QWidget* parent) : QMainWindow(parent), ui(new Ui::QGlk), mp_Runnable(new Glk::Runnable()), mp_RootWindow(NULL), m_InterruptHandler(), m_DefaultStyles(), m_TextBufferStyles() {
    setMinimumSize(600, 400);
    ui->setupUi(this);

    mp_Runnable->setAutoDelete(true);
}

QGlk::~QGlk() {
    delete ui;
}

void QGlk::run() {
    QThreadPool::globalInstance()->start(mp_Runnable);
}

bool QGlk::event(QEvent* event) {
    if(event->type() == Glk::TaskEvent::Type)
        return handleGlkTask(static_cast<Glk::TaskEvent*>(event));
    else
        return QMainWindow::event(event);
}

void QGlk::closeEvent(QCloseEvent* event) {
    m_EventQueue.interrupt();
    event->accept();
}

void QGlk::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    eventQueue().push(event_t {evtype_Arrange, NULL, 0, 0});
}

bool QGlk::handleGlkTask(Glk::TaskEvent* event) {
    event->execute();

    return true;
}
