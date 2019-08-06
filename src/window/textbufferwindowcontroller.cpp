#include "textbufferwindowcontroller.hpp"

#include <QSemaphore>
#include <QThread>

#include "qglk.hpp"
#include "log/log.hpp"
#include "thread/taskrequest.hpp"

#include "textbufferwidget.hpp"
#include "textbufferwindow.hpp"

Glk::TextBufferWindowController::TextBufferWindowController(Glk::PairWindow* winParent, glui32 winRock)
    : WindowController(new TextBufferWindow(this, winParent, winRock), createWidget()),
      mp_EventThreadDocument{nullptr},
      mp_Cursor{nullptr} {
    Glk::sendTaskToEventThread([this]() {
        mp_EventThreadDocument = new QTextDocument;
        mp_Cursor = new QTextCursor{mp_EventThreadDocument};
    });
}

Glk::TextBufferWindowController::~TextBufferWindowController() {
    Glk::sendTaskToEventThread([this]() {
        delete mp_Cursor;
        delete mp_EventThreadDocument;
    });
}

void Glk::TextBufferWindowController::cancelCharInput() {
    assert(onGlkThread());

    if(charInputRequest() && !charInputRequest()->isCancelled()) {
        charInputRequest()->cancel();
        requestSynchronization();
    }
}

event_t Glk::TextBufferWindowController::cancelLineInput() {
    assert(onGlkThread());

    if(lineInputRequest() && !lineInputRequest()->isCancelled()) {
        // horrible hack
        QSemaphore sem;
        QObject context;
        event_t cancelEvent;

        QObject::connect(lineInputRequest(), &LineInputRequest::fulfilled, &context, [this, &sem, &cancelEvent]() {
            cancelEvent = lineInputRequest()->generateEvent(window());
            sem.release();
        }, Qt::DirectConnection);

        lineInputRequest()->cancel();

        // block until fulfilled signal is emitted
        sem.acquire();

        requestSynchronization();

        return cancelEvent;
    } else {
        return {evtype_None, nullptr, 0, 0};
    }
}

void Glk::TextBufferWindowController::requestCharInput(bool unicode) {
    assert(onGlkThread());
    assert(!lineInputRequest() || lineInputRequest()->isCancelled() || lineInputRequest()->isFulfilled());
    assert(!charInputRequest() || charInputRequest()->isCancelled());

    setCharInputRequest(new CharInputRequest(unicode));

    requestSynchronization();
}

void Glk::TextBufferWindowController::requestLineInput(void* buf, glui32 maxLen, glui32 initLen, bool unicode) {
    assert(onGlkThread());
    assert(!charInputRequest() || charInputRequest()->isCancelled() || charInputRequest()->isFulfilled());
    assert(!lineInputRequest() || lineInputRequest()->isCancelled());

    setLineInputRequest(new LineInputRequest(buf, maxLen, initLen, unicode, lineInputTerminators(), lineInputEchoes()));

    requestSynchronization();
}

void Glk::TextBufferWindowController::synchronize() {
    assert(onEventThread());

    if(charInputRequest()) {
        if(charInputRequest()->isFulfilled() || charInputRequest()->isCancelled()) {
            setCharInputRequest(nullptr);
        } else if(!widget<TextBufferWidget>()->charInputPending()) {
            widget<TextBufferWidget>()->requestCharInput();

            QObject::connect(widget<TextBufferWidget>(), &TextBufferWidget::characterInput,
                             charInputRequest(), &CharInputRequest::fulfill, Qt::DirectConnection);

            QObject::connect(charInputRequest(), &CharInputRequest::fulfilled, [this]() {
                QGlk::getMainWindow().eventQueue().push(charInputRequest()->generateEvent(window()));
                requestSynchronization();
            });

            QObject::connect(charInputRequest(), &CharInputRequest::cancelled,
                             widget<TextBufferWidget>(), &TextBufferWidget::cancelCharInput, Qt::QueuedConnection);
        }

        // input pending but still not fulfilled or cancelled: do nothing
    }

    if(lineInputRequest()) {
        if(lineInputRequest()->isFulfilled() || lineInputRequest()->isCancelled()) {
            if(lineInputRequest()->lineEchoes())
                window<TextBufferWindow>()->writeString(lineInputRequest()->text() + '\n');
            setLineInputRequest(nullptr);
        } else if(!widget<TextBufferWidget>()->lineInputPending()) {
            synchronizeText();

            auto tbBrowser = widget<TextBufferWidget>()->browser();

            Style style = window<TextBufferWindow>()->styles()[Style::Input];
            QTextCursor tbBrowserCursor{widget<TextBufferWidget>()->browser()->document()};
            tbBrowserCursor.movePosition(QTextCursor::End);
            tbBrowserCursor.setBlockFormat(style.blockFormat());
            tbBrowserCursor.setCharFormat(style.charFormat());

            tbBrowser->setTextCursor(tbBrowserCursor);

            widget<TextBufferWidget>()->requestLineInput(lineInputRequest()->bufferLength(),
                                                         lineInputRequest()->lineTerminators());

            QObject::connect(widget<TextBufferWidget>(), &TextBufferWidget::lineInput,
                             lineInputRequest(), &LineInputRequest::fulfill, Qt::DirectConnection);

            QObject::connect(lineInputRequest(), &LineInputRequest::fulfilled, [this]() {
                QGlk::getMainWindow().eventQueue().push(lineInputRequest()->generateEvent(window()));
                requestSynchronization();
            });

            QObject::connect(lineInputRequest(), &LineInputRequest::cancelled,
                             widget<TextBufferWidget>(), &TextBufferWidget::cancelLineInput, Qt::QueuedConnection);

            WindowController::synchronize();

            return;
        }

        // input pending but still not fulfilled or cancelled: do nothing
    }

    // TODO hyperlink request

    synchronizeText();

    WindowController::synchronize();
}

//void Glk::TextBufferWindowController::synchronize() {
//    assert(onEventThread());
//
//    if(charInputRequest()) {
//        if(charInputRequest()->isFulfilled() || charInputRequest()->isCancelled()) {
//            setCharInputRequest(nullptr);
//        } else if(!widget<TextBufferWidget>()->browser()->charInputPending()) {
//            auto tbBrowser = widget<TextBufferWidget>()->browser();
//
//            tbBrowser->requestCharInput();
//            QObject::connect(tbBrowser, &TextBufferBrowser::characterInput, charInputRequest(),
//                             &CharInputRequest::fulfill, Qt::DirectConnection);
//            QObject::connect(charInputRequest(), &CharInputRequest::fulfilled, [this]() {
//                QGlk::getMainWindow().eventQueue().push(charInputRequest()->generateEvent(window()));
//                requestSynchronization();
//            });
//            QObject::connect(charInputRequest(), &CharInputRequest::cancelled, tbBrowser,
//                             &TextBufferBrowser::onCharInputRequestCancelled, Qt::QueuedConnection);
//        }
//
//        // input pending but still not fulfilled or cancelled: do nothing
//    }
//
//    if(lineInputRequest()) {
//        if(lineInputRequest()->isFulfilled()) {
//            if(lineInputRequest()->lineEchoes())
//                window<TextBufferWindow>()->writeString(lineInputRequest()->text() + '\n');
//            setLineInputRequest(nullptr);
//        } else if(lineInputRequest()->isCancelled()) {
//            setLineInputRequest(nullptr);
//        } else if(!widget<TextBufferWidget>()->browser()->lineInputPending()) {
//            synchronizeText();
//
//            auto tbBrowser = widget<TextBufferWidget>()->browser();
//
//            Style style = window<TextBufferWindow>()->styles()[Style::Input];
//            QTextCursor tbBrowserCursor{widget<TextBufferWidget>()->browser()->document()};
//            tbBrowserCursor.movePosition(QTextCursor::End);
//            tbBrowserCursor.setBlockFormat(style.blockFormat());
//            tbBrowserCursor.setCharFormat(style.charFormat());
//
//            tbBrowser->setTextCursor(tbBrowserCursor);
//
//            tbBrowser->requestLineInput();
//            QObject::connect(tbBrowser, &TextBufferBrowser::lineInput, lineInputRequest(),
//                             &LineInputRequest::fulfill, Qt::DirectConnection);
//            QObject::connect(lineInputRequest(), &LineInputRequest::fulfilled, [this]() {
//                QGlk::getMainWindow().eventQueue().push(lineInputRequest()->generateEvent(window()));
//                requestSynchronization();
//            });
//            QObject::connect(lineInputRequest(), &LineInputRequest::cancelled, tbBrowser,
//                             &TextBufferBrowser::onLineInputRequestCancelled, Qt::QueuedConnection);
//
//            WindowController::synchronize();
//
//            return;
//        }
//
//        // input pending but still not fulfilled or cancelled: do nothing
//    }
//
//    // TODO hyperlink request
//
//    synchronizeText();
//
//    WindowController::synchronize();
//}

QPoint Glk::TextBufferWindowController::glkPos(const QPoint& qtPos) const {
    log_warn() << "Requesting glk position of Qt point (" << qtPos.x() << ", " << qtPos.y() << ") for window " << TO_WINID(window()) << "(" << Window::windowsTypeString(window()->windowType()) << ")";

    return qtPos;
}

QSize Glk::TextBufferWindowController::glkSize() const {
    QRect widgetBrowserFrameRect = widget<TextBufferWidget>()->browser()->frameRect();
    return {widgetBrowserFrameRect.width() / widget()->fontMetrics().horizontalAdvance('m'),
            widgetBrowserFrameRect.height() / widget()->fontMetrics().height()};
}

QSize Glk::TextBufferWindowController::toQtSize(const QSize& glk) const {
    int w = glk.width() * widget()->fontMetrics().horizontalAdvance('0');
    int h = glk.height() * widget()->fontMetrics().height();

    return {w + widget()->contentsMargins().left() + widget()->contentsMargins().right(),
            h + widget()->contentsMargins().top() + widget()->contentsMargins().bottom()};
}

void Glk::TextBufferWindowController::clearDocument() {
    Glk::postTaskToEventThread([this]() {
        mp_EventThreadDocument->clear();
        mp_Cursor->movePosition(QTextCursor::Start);
        requestSynchronization();
    });
}

void Glk::TextBufferWindowController::flowBreak() {
    Glk::postTaskToEventThread([this]() {
        mp_Cursor->insertBlock();
        requestSynchronization();
    });
}

void Glk::TextBufferWindowController::writeHTML(const QString& html) {
    Glk::postTaskToEventThread([this, html]() {
        mp_Cursor->insertHtml(html);
        requestSynchronization();
    });
}

void Glk::TextBufferWindowController::writeString(const QString& str, const QTextCharFormat& chFmt,
                                                  const QTextBlockFormat& blkFmt) {
    Glk::postTaskToEventThread([this, str, chFmt, blkFmt]() {
        mp_Cursor->setBlockFormat(blkFmt);
        mp_Cursor->insertText(str, chFmt);
        requestSynchronization();
    });
}

QWidget* Glk::TextBufferWindowController::createWidget() {
    QWidget* w = nullptr;

    Glk::sendTaskToEventThread([&w]() {
        w = new TextBufferWidget;
        w->hide();
    });

    return w;
}

void Glk::TextBufferWindowController::synchronizeText() {
    widget<TextBufferWidget>()->browser()->setImages(window<TextBufferWindow>()->images());

    auto html = mp_EventThreadDocument->toHtml();
    widget<TextBufferWidget>()->browser()->setHtml(html);

    widget<TextBufferWidget>()->browser()->moveCursor(QTextCursor::End);
}
