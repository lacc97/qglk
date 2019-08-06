#include "textbufferwidget.hpp"

#include <QGridLayout>
#include <QKeyEvent>

#include "log/log.hpp"
#include "thread/taskrequest.hpp"

Glk::TextBufferBrowser::TextBufferBrowser(Glk::TextBufferWidget* wParent)
    : QTextBrowser{wParent},
      m_Images{},
      m_LineInputStartCursorPosition{-1} {
    setReadOnly(true);
    setOpenExternalLinks(false);
    setOpenLinks(false);

    connect(this, &TextBufferBrowser::cursorPositionChanged, this, &TextBufferBrowser::onCursorPositionChanged);
}

QVariant Glk::TextBufferBrowser::loadResource(int type, const QUrl& name) {
    if(type == QTextDocument::ImageResource) {
        int imgIndex = name.toString().toUInt();

        if(imgIndex < m_Images.size())
            return QVariant::fromValue(m_Images[imgIndex]);
    }

    return QTextBrowser::loadResource(type, name);
}

void Glk::TextBufferBrowser::onCursorPositionChanged() {
    if(m_LineInputStartCursorPosition >= 0 &&
       m_LineInputStartCursorPosition > textCursor().position()) {
        auto c = textCursor();
        c.setPosition(m_LineInputStartCursorPosition,
                      c.hasSelection() ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        setTextCursor(c);
    }
}

void Glk::TextBufferBrowser::keyPressEvent(QKeyEvent* ev) {
    if(m_LineInputStartCursorPosition >= 0 && textCursor().position() == m_LineInputStartCursorPosition &&
       ev->key() == Qt::Key_Backspace) // This ensures we can't delete anything that came before the line input request.
        return;

    return QTextBrowser::keyPressEvent(ev);
}

Glk::TextBufferWidget::TextBufferWidget()
    : WindowWidget{},
      mp_Browser{nullptr} {
    assert(onEventThread());

    QLayout* lay = new QGridLayout(this);
    lay->setMargin(0);

    mp_Browser = new TextBufferBrowser(this);
    mp_Browser->setReadOnly(true);
    mp_Browser->setOpenExternalLinks(false);
    mp_Browser->setOpenLinks(false);

    lay->addWidget(mp_Browser);


    installInputFilter(mp_Browser);
}

QString Glk::TextBufferWidget::lineInputBuffer() {
    auto c = browser()->textCursor();
    c.setPosition(browser()->lineInputStartCursorPosition());
    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

    QString inputText = c.selectedText();
    assert(!inputText.contains(0x2029));

    return inputText;
}

void Glk::TextBufferWidget::onLineInputRequested() {
    browser()->setReadOnly(false);

    browser()->moveCursor(QTextCursor::End);
    browser()->setLineInputStartCursorPosition(browser()->textCursor().position());
}

void Glk::TextBufferWidget::onLineInputFinished() {
    browser()->setReadOnly(true);

    browser()->setLineInputStartCursorPosition(-1);
}


//#include "textbufferwidget.hpp"
//
//#include <QGridLayout>
//#include <QKeyEvent>
//
//#include "log/log.hpp"
//#include "thread/taskrequest.hpp"
//
//Glk::TextBufferBrowser::TextBufferBrowser(Glk::TextBufferWidget* wParent)
//    : QTextBrowser{wParent},
//      m_Images{},
//      m_ReceivingCharInput{false},
//      m_ReceivingLineInput{false},
//      m_LineTerminators{},
//      m_LineInputStartCursorPosition{0} {
//    setReadOnly(true);
//    setOpenExternalLinks(false);
//    setOpenLinks(false);
//
//    connect(this, &TextBufferBrowser::cursorPositionChanged, this, &TextBufferBrowser::onCursorPositionChanged);
//}
//
//QVariant Glk::TextBufferBrowser::loadResource(int type, const QUrl& name) {
//    if(type == QTextDocument::ImageResource) {
//        int imgIndex = name.toString().toUInt();
//
//        if(imgIndex < m_Images.size())
//            return QVariant::fromValue(m_Images[imgIndex]);
//    }
//
//    return QTextBrowser::loadResource(type, name);
//}
//
//
//void Glk::TextBufferBrowser::requestCharInput() {
//    assert(onEventThread());
//    assert(!m_ReceivingLineInput);
//    assert(!m_ReceivingCharInput);
//
//    m_ReceivingCharInput = true;
//    setFocus();
//}
//
//void Glk::TextBufferBrowser::requestLineInput() {
//    assert(onEventThread());
//    assert(!m_ReceivingCharInput);
//    assert(!m_ReceivingLineInput);
//
//    m_ReceivingLineInput = true;
//    setFocus();
//    setReadOnly(false);
//
//    moveCursor(QTextCursor::End);
//
//    m_LineInputStartCursorPosition = textCursor().position();
//}
//
//void Glk::TextBufferBrowser::onCharInputRequestCancelled() {
//    assert(onEventThread());
//
//    if(m_ReceivingCharInput)
//        endCharInputRequest();
//}
//
//void Glk::TextBufferBrowser::onCursorPositionChanged() {
//    if(m_ReceivingLineInput && m_LineInputStartCursorPosition > textCursor().position()) {
//        auto c = textCursor();
//        c.setPosition(m_LineInputStartCursorPosition,
//                      c.hasSelection() ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
//        setTextCursor(c);
//    }
//}
//
//void Glk::TextBufferBrowser::onLineInputRequestCancelled() {
//    assert(onEventThread());
//
//    if(m_ReceivingLineInput)
//        endLineInputRequest(Qt::Key_Enter);
//}
//
//void Glk::TextBufferBrowser::endCharInputRequest() {
//    m_ReceivingCharInput = false;
//}
//
//void Glk::TextBufferBrowser::endLineInputRequest(Qt::Key terminator) {
//    m_ReceivingLineInput = false;
//
//    auto c = textCursor();
//    c.setPosition(m_LineInputStartCursorPosition);
//    c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
//
//    QString inputText = c.selectedText();
//    assert(!inputText.contains(0x2029));
//
//    emit lineInput(terminator, inputText);
//
//    setReadOnly(true);
//}
//
//void Glk::TextBufferBrowser::keyPressEvent(QKeyEvent* ev) {
//    if(m_ReceivingCharInput) {
//        emit characterInput(static_cast<Qt::Key>(ev->key()), ev->text());
//        endCharInputRequest();
//        return;
//    } else if(m_ReceivingLineInput) {
//        if(ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter ||
//           m_LineTerminators.find(static_cast<Qt::Key>(ev->key())) != m_LineTerminators.end()) {
//            endLineInputRequest(static_cast<Qt::Key>(ev->key()));
//            return;
//        } else if(textCursor().position() == m_LineInputStartCursorPosition && ev->key() == Qt::Key_Backspace) {
//            // This ensures we can't delete anything that came before the line input request.
//            return;
//        }
//    }
//
//    QTextBrowser::keyPressEvent(ev);
//}
//
//Glk::TextBufferWidget::TextBufferWidget()
//    : QWidget{},
//      mp_Browser{nullptr} {
//    assert(onEventThread());
//
//    QLayout* lay = new QGridLayout(this);
//    lay->setMargin(0);
//
//    mp_Browser = new TextBufferBrowser(this);
//    mp_Browser->setReadOnly(true);
//    mp_Browser->setOpenExternalLinks(false);
//    mp_Browser->setOpenLinks(false);
//
//    lay->addWidget(mp_Browser);
//}
