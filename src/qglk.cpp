#include "qglk.hpp"
#include "ui_qglk.h"

#include <QResizeEvent>

#include "glk.hpp"

QGlk::QGlk(QWidget* parent) : QMainWindow(parent), ui(new Ui::QGlk), mp_RootWindow(NULL) {
    setMinimumSize(600, 400);
    ui->setupUi(this);
}

QGlk::~QGlk() {
    delete ui;
}

bool QGlk::event(QEvent* event) {
    if(event->type() == Glk::TaskEvent::Type)
        return handleGlkTask(static_cast<Glk::TaskEvent*>(event));
    else
        return QMainWindow::event(event);
}

void QGlk::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);
    
    eventQueue().push(event_t{evtype_Arrange, NULL, 0, 0});
}

bool QGlk::handleGlkTask(Glk::TaskEvent* event) {
    event->execute();

    return true;
}
