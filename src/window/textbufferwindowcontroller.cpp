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

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequested,
                     [this]() {
                         assert(onEventThread());

                         auto tbBrowser = widget<TextBufferWidget>()->browser();

                         Style style = window<TextBufferWindow>()->styles()[Style::Input];
                         QTextCursor tbBrowserCursor{widget<TextBufferWidget>()->browser()->document()};
                         tbBrowserCursor.movePosition(QTextCursor::End);
                         tbBrowserCursor.setBlockFormat(style.blockFormat());
                         tbBrowserCursor.setCharFormat(style.charFormat());

                         tbBrowser->setTextCursor(tbBrowserCursor);
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequestCancelled,
                     [this](const QString& text, bool lineEchoes) {
                         if(lineEchoes && !text.isEmpty())
                             window<TextBufferWindow>()->writeString(text + '\n');

                         requestSynchronization();
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequestFulfilled,
                     [this](const QString& text, bool lineEchoes) {
                         if(lineEchoes && !text.isEmpty())
                             window<TextBufferWindow>()->writeString(text + '\n');

                         requestSynchronization();
                     });
}

Glk::TextBufferWindowController::~TextBufferWindowController() {
    Glk::sendTaskToEventThread([this]() {
        delete mp_Cursor;
        delete mp_EventThreadDocument;
    });
}

void Glk::TextBufferWindowController::synchronize() {
    assert(onEventThread());

    if(!keyboardProvider()->lineInputRequest() || !keyboardProvider()->lineInputRequest()->isPending())
        synchronizeText();

    WindowController::synchronize();
}

QPoint Glk::TextBufferWindowController::glkPos(const QPoint& qtPos) const {
    spdlog::warn("Requesting glk position of Qt point ({}, {}) for window {}", qtPos.x(), qtPos.y(), wrap::ptr(window()));

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

bool Glk::TextBufferWindowController::supportsCharInput() const {
    return true;
}

bool Glk::TextBufferWindowController::supportsHyperlinkInput() const {
    return true;
}

bool Glk::TextBufferWindowController::supportsLineInput() const {
    return true;
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
    assert(onEventThread());

    widget<TextBufferWidget>()->browser()->setImages(window<TextBufferWindow>()->images());

    widget<TextBufferWidget>()->browser()->setHtml(mp_EventThreadDocument->toHtml());

    widget<TextBufferWidget>()->browser()->moveCursor(QTextCursor::End);
}
