#include "textgridwindowcontroller.hpp"

#include "qglk.hpp"
#include "log/log.hpp"
#include "thread/taskrequest.hpp"

#include "textgridwidget.hpp"
#include "textgridwindow.hpp"

Glk::TextGridWindowController::TextGridWindowController(Glk::PairWindow* winParent, glui32 winRock)
    : WindowController(
    new TextGridWindow(this, winParent, winRock), createWidget()) {
    widget<TextGridWidget>()->setGrid(window<TextGridWindow>()->grid());

    QObject::connect(widget<TextGridWidget>(), &TextGridWidget::resized, [this]() {
        requestSynchronization();
        QGlk::getMainWindow().eventQueue().requestImmediateSynchronization();
    });
}

bool Glk::TextGridWindowController::supportsCharInput() const {
    return true;
}

void Glk::TextGridWindowController::synchronize() {
    assert(onEventThread());

    widget<TextGridWidget>()->setGrid(window<TextGridWindow>()->grid());

    if(widget()->isVisible() && (widget()->width() != 0 || widget()->height() != 0)) {
        QRect widgetContentsRect = widget()->contentsRect();
        QSize widgetGlkSize(widgetContentsRect.width() / widget()->fontMetrics().horizontalAdvance('m'),
                            widgetContentsRect.height() / widget()->fontMetrics().height());

        if(widgetGlkSize != window<TextGridWindow>()->gridSize()) {
            window<TextGridWindow>()->resizeGrid(widgetGlkSize);
            QGlk::getMainWindow().eventQueue().push(event_t{evtype_Arrange, TO_WINID(window())});
        }
    }

    WindowController::synchronize();
}

QPoint Glk::TextGridWindowController::glkPos(const QPoint& qtPos) const {
    spdlog::warn("Requesting glk position of Qt point ({}, {}) for window {}", qtPos.x(), qtPos.y(), wrap::ptr(window()));

    return qtPos;
}

QSize Glk::TextGridWindowController::glkSize() const {
    return window<TextGridWindow>()->gridSize();
}

//QSize Glk::TextGridWindowController::toGlkUnits(QSize qtUnits) {
//    QRect widgetContentsRect = widget()->contentsRect();
//
//    int effectiveWidth = std::clamp(qtUnits.width() - hMargins, 0, widget()->size().width() - hMargins);
//    int effectiveHeight = std::clamp(qtUnits.height() - vMargins, 0, widget()->size().height() - vMargins);
//
//    return QSize(effectiveWidth / widget()->fontMetrics().horizontalAdvance('m'),
//                 effectiveHeight / widget()->fontMetrics().height());
//}

QSize Glk::TextGridWindowController::toQtSize(const QSize& glk) const {
    int w = glk.width() * widget()->fontMetrics().horizontalAdvance('m');
    int h = glk.height() * widget()->fontMetrics().height();

    return {w + widget()->contentsMargins().left() + widget()->contentsMargins().right(),
            h + widget()->contentsMargins().top() + widget()->contentsMargins().bottom()};
}

QWidget* Glk::TextGridWindowController::createWidget() {
    QWidget* w = nullptr;

    Glk::sendTaskToEventThread([&w]() {
        w = new TextGridWidget;
        w->hide();
    });

    return w;
}
