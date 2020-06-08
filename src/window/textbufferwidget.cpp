#include "textbufferwidget.hpp"

#include <QGridLayout>
#include <QKeyEvent>

#include "log/log.hpp"
#include "thread/taskrequest.hpp"

Glk::TextBufferBrowser::History::History()
        : m_History{} {}

void Glk::TextBufferBrowser::History::push(const QString& newcmd) {
    if(m_History.size() > MAX_SIZE)
        m_History.pop_back();

    m_History.push_front(newcmd);
}

Glk::TextBufferBrowser::TextBufferBrowser(Glk::TextBufferWidget* wParent)
    : QTextBrowser{wParent},
      m_Images{},
      m_LineInputStartCursorPosition{-1},
      m_History{},
      m_HistoryIterator{m_History.begin()} {
    connect(wParent, &TextBufferWidget::lineInput, [this](Qt::Key, const QString& input) {
        m_History.push(input);
        m_HistoryIterator = m_History.begin();
    });

    connect(this, &TextBufferBrowser::cursorPositionChanged,
            this, &TextBufferBrowser::onCursorPositionChanged);
}

QString Glk::TextBufferBrowser::lineInputBuffer() const {
    if(receivingLineInput()) {
        auto c = textCursor();

        c.setPosition(lineInputStartCursorPosition());
        c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);

        assert(!c.selectedText().contains(0x2029));
        return c.selectedText();
    } else {
        return {};
    }
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
    if(receivingLineInput() && m_LineInputStartCursorPosition > textCursor().position()) {
        auto c = textCursor();
        c.setPosition(m_LineInputStartCursorPosition,
                      c.hasSelection() ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
        setTextCursor(c);
    }
}

void Glk::TextBufferBrowser::keyPressEvent(QKeyEvent* ev) {
    if(receivingLineInput()) {
        if(textCursor().position() == m_LineInputStartCursorPosition && ev->key() == Qt::Key_Backspace) {
            // This ensures we can't delete anything that came before the line input request.
            return;
        }

        if(ev->key() == Qt::Key_Up) {
            if(m_HistoryIterator != m_History.end()) {
                setLineInputBuffer(*m_HistoryIterator++);
            }

            return;
        } else if(ev->key() == Qt::Key_Down) {
            if(m_HistoryIterator != m_History.begin()) {
                setLineInputBuffer(*(--m_HistoryIterator));
            } else {
                setLineInputBuffer({});
            }

            return;
        }
    }

    return QTextBrowser::keyPressEvent(ev);
}

void Glk::TextBufferBrowser::setLineInputBuffer(const QString& str) {
    if(receivingLineInput()) {
        auto c = textCursor();

        c.setPosition(lineInputStartCursorPosition());
        c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        c.removeSelectedText();

        c.setPosition(lineInputStartCursorPosition());
        c.insertText(str);
    }
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
    return browser()->lineInputBuffer();
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
