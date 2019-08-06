#include "blankwindowcontroller.hpp"

#include "event/eventqueue.hpp"

#include "blankwindow.hpp"


Glk::BlankWindowController::BlankWindowController(PairWindow* parent, glui32 rock)
    : WindowController(new BlankWindow(this, parent, rock), createWidget()) {}

QPoint Glk::BlankWindowController::glkPos(const QPoint& qtPos) const {
    return qtPos;
}

QSize Glk::BlankWindowController::glkSize() const {
    return widget()->size();
}

QSize Glk::BlankWindowController::toQtSize(const QSize& glk) const {
    return glk;
}

QWidget* Glk::BlankWindowController::createWidget() {
    QWidget* w = nullptr;

    Glk::sendTaskToEventThread([&w]() {
        w = new QWidget;
        w->hide();
    });

    return w;
}
