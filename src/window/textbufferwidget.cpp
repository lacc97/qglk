#include "textbufferwidget.hpp"

#include <QGridLayout>
#include <QKeyEvent>

#include "log/log.hpp"
#include "thread/taskrequest.hpp"

Glk::TextBufferBrowser::TextBufferBrowser(Glk::TextBufferWidget* wParent)
    : QTextBrowser{wParent},
      m_Images{},
      m_LineInputStartCursorPosition{-1} {
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
    mp_Browser->setOpenLinks(true);

    QObject::connect(mp_Browser, &TextBufferBrowser::anchorClicked,
                     this, &TextBufferWidget::onHyperlinkPressed);

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

void Glk::TextBufferWidget::onHyperlinkPressed(const QUrl& url) {
    if(hyperlinkInputPending()) {
        bool success = false;
        uint linkval = url.toString().toUInt(&success);

        if(success)
            emit hyperlinkInput(linkval);
        else
            spdlog::warn("Received rubbish hyperlink input {0} at TextBufferWidget ({1})", url.toString(), (void*)this);
    }
}
