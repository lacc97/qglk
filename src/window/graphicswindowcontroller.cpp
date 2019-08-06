#include "graphicswindowcontroller.hpp"

#include <QPainter>

#include "thread/taskrequest.hpp"


#include "qglk.hpp"

#include "graphicswidget.hpp"
#include "graphicswindow.hpp"


Glk::GraphicsWindowController::GraphicsWindowController(Glk::PairWindow* parent, glui32 rock)
    : WindowController(new GraphicsWindow(this, parent, rock), createWidget()) {
    window<GraphicsWindow>()->setBackgroundColor(getDefaultBackgroundColor());

    QObject::connect(widget<GraphicsWidget>(), &GraphicsWidget::resized, [this]() {
        requestSynchronization();
    });
}

void Glk::GraphicsWindowController::cancelMouseInput() {
    assert(onGlkThread());

    if(mouseInputRequest() && !mouseInputRequest()->isCancelled()) {
        mouseInputRequest()->cancel();
        requestSynchronization();
    }
}

void Glk::GraphicsWindowController::requestMouseInput() {
    assert(onGlkThread());
    assert(!mouseInputRequest() || mouseInputRequest()->isCancelled());

    setMouseInputRequest(new MouseInputRequest);

    requestSynchronization();
}

void Glk::GraphicsWindowController::synchronize() {
    assert(onEventThread());

    if(mouseInputRequest()) {
        if(mouseInputRequest()->isFulfilled() || mouseInputRequest()->isCancelled()) {
            setMouseInputRequest(nullptr);
        } else if(!widget<GraphicsWidget>()->mouseInputPending()) {
            widget<GraphicsWidget>()->requestMouseInput();

            QObject::connect(widget<GraphicsWidget>(), &GraphicsWidget::mouseInput,
                             mouseInputRequest(), &MouseInputRequest::fulfill, Qt::DirectConnection);

            QObject::connect(mouseInputRequest(), &MouseInputRequest::fulfilled, [this]() {
                QGlk::getMainWindow().eventQueue().push(mouseInputRequest()->generateEvent(window()));
                requestSynchronization();
            });

            QObject::connect(mouseInputRequest(), &MouseInputRequest::cancelled,
                             widget<GraphicsWidget>(), &GraphicsWidget::cancelMouseInput, Qt::QueuedConnection);
        }

        // input pending but still not fulfilled or cancelled: do nothing
    }

    if(widget<GraphicsWidget>()->backgroundColor() != window<GraphicsWindow>()->backgroundColor())
        widget<GraphicsWidget>()->setBackgroundColor(window<GraphicsWindow>()->backgroundColor());

    QSize clampedWidgetSize = {std::max(1, widget<GraphicsWidget>()->buffer().width()),
                               std::max(1, widget<GraphicsWidget>()->buffer().height())};

    if(clampedWidgetSize != window<GraphicsWindow>()->buffer().size()) {
        window<GraphicsWindow>()->resizeBuffer(widget<GraphicsWidget>()->buffer().size());
        QGlk::getMainWindow().eventQueue().push(event_t{evtype_Arrange, TO_WINID(window())});
    }

    widget<GraphicsWidget>()->setBuffer(window<GraphicsWindow>()->buffer());

    WindowController::synchronize();
}

QPoint Glk::GraphicsWindowController::glkPos(const QPoint& qtPos) const {
    return qtPos;
}

QSize Glk::GraphicsWindowController::glkSize() const {
    return window<GraphicsWindow>()->buffer().size();
}

QSize Glk::GraphicsWindowController::toQtSize(const QSize& glk) const {
    return glk;
}

QColor Glk::GraphicsWindowController::getDefaultBackgroundColor() const {
    return widget<GraphicsWidget>()->getDefaultBackgroundColor();
}

QWidget* Glk::GraphicsWindowController::createWidget() {
    QWidget* w = nullptr;

    Glk::sendTaskToEventThread([&w]() {
        w = new GraphicsWidget;
        w->hide();
    });

    return w;
}
