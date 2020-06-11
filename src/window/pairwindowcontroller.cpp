#include "pairwindowcontroller.hpp"

#include "log/log.hpp"
#include "thread/taskrequest.hpp"

#include "pairwidget.hpp"
#include "pairwindow.hpp"

Glk::PairWindowController* Glk::PairWindowController::createController(Glk::Window* winKey, Glk::Window* winFirst,
                                                                       Glk::Window* winSecond,
                                                                       Glk::WindowArrangement* winArrangement,
                                                                       Glk::PairWindow* winParent) {
    return new PairWindowController(winKey, winFirst, winSecond, winArrangement, winParent);
}

Glk::PairWindowController::PairWindowController(Glk::Window* winKey, Glk::Window* winFirst, Glk::Window* winSecond,
                                                Glk::WindowArrangement* winArrangement, Glk::PairWindow* winParent)
    : WindowController{new PairWindow(winKey, winFirst, winSecond, winArrangement, this, winParent),
                       createWidget()} {

}

QPoint Glk::PairWindowController::glkPos(const QPoint& qtPos) const {
    spdlog::warn("Requesting glk position of Qt point ({}, {}) for window {}", qtPos.x(), qtPos.y(), wrap::ptr(window()));

    return qtPos;
}

QSize Glk::PairWindowController::glkSize() const {
    return widget()->size();
}

QSize Glk::PairWindowController::toQtSize(const QSize& glk) const {
    return glk;
}

void Glk::PairWindowController::closeWindow() {
    assert(onGlkThread());

    auto firstWin = window<Glk::PairWindow>()->firstWindow();
    auto secondWin = window<Glk::PairWindow>()->secondWindow();

    // This ensures the window subtree is deleted from the bottom up
    if(firstWin)
        firstWin->controller()->closeWindow();

    if(secondWin)
        secondWin->controller()->closeWindow();

    WindowController::closeWindow();
}

void Glk::PairWindowController::synchronize() {
    assert(onEventThread());

    window<PairWindow>()->arrangement()->setupWidgets(this);

    auto firstWin = window<Glk::PairWindow>()->firstWindow();
    auto secondWin = window<Glk::PairWindow>()->secondWindow();

    if(firstWin) {
        firstWin->controller()->synchronize();
        assert(!firstWin->controller()->requiresSynchronization());
    }

    if(secondWin) {
        secondWin->controller()->synchronize();
        assert(!secondWin->controller()->requiresSynchronization());
    }

    WindowController::synchronize();
}

QWidget* Glk::PairWindowController::createWidget() {
    QWidget* w = nullptr;

    Glk::sendTaskToEventThread([&w]() {
        w = new PairWidget;
        w->hide();
    });

    return w;
}
