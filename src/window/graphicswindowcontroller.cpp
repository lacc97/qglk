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

bool Glk::GraphicsWindowController::supportsMouseInput() const {
    return true;
}

void Glk::GraphicsWindowController::synchronize() {
    assert(onEventThread());

    if(widget<GraphicsWidget>()->backgroundColor() != window<GraphicsWindow>()->backgroundColor())
        widget<GraphicsWidget>()->setBackgroundColor(window<GraphicsWindow>()->backgroundColor());

    QSize clampedWidgetSize = {std::max(1, widget<GraphicsWidget>()->contentsRect().width()),
                               std::max(1, widget<GraphicsWidget>()->contentsRect().height())};

    if(clampedWidgetSize != window<GraphicsWindow>()->buffer().size()) {
        window<GraphicsWindow>()->resizeBuffer(clampedWidgetSize);
        QGlk::getMainWindow().eventQueue().push(event_t{evtype_Arrange, TO_WINID(window())});
    }

    widget<GraphicsWidget>()->setBuffer(QPixmap::fromImage(window<GraphicsWindow>()->buffer()));
    widget<GraphicsWidget>()->update();

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
