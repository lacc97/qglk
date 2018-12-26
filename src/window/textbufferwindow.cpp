#include "textbufferwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollBar>

#include "qglk.hpp"

void Glk::TextBufferDevice::Block::appendWords(QStringList words, const QString& styleString) {
    Q_ASSERT_X(!words.isEmpty(), "Glk::TextBufferDevice::Block::appendWords", "word list should not be empty");

    if(m_Words.isEmpty())
        m_Words.append(QStringLiteral("<span style=\"%1\">%2</span>").arg(styleString).arg(words.front()));
    else
        m_Words.back().append(QStringLiteral("<span style=\"%1\">%2</span>").arg(styleString).arg(words.front()));

    words.pop_front();

    for(const QString& w : words)
        m_Words.append(QStringLiteral("<span style=\"%1\">%2</span>").arg(styleString).arg(w));
}

void Glk::TextBufferDevice::Block::insertOpenHyperlinkTag(glui32 linkval) {
    if(m_Words.isEmpty())
        m_Words.push_back(QStringLiteral("<a href=\"#%1\">").arg(QString::number(linkval)));
    else
        m_Words.back().append(QStringLiteral("<a href=\"#%1\">").arg(QString::number(linkval)));
}

void Glk::TextBufferDevice::Block::insertCloseHyperlinkTag() {
    Q_ASSERT_X(!m_Words.isEmpty(), "Glk::TextBufferDevice::Block::insertCloseHyperlinkTag", "if hyperlink tag is till open when flushing, we need to ensure a new closing and opening tag is appended");
    
    m_Words.back().append(QStringLiteral("</a>"));
}

void Glk::TextBufferDevice::Block::writeToBrowser(QTextBrowser* qtb) const {
    Q_ASSERT_X(!m_Words.isEmpty(), "Glk::TextBufferDevice::Block::writeToBrowser", "word list should not be empty");

    qtb->insertHtml(m_Words.front());

    for(auto it = (++m_Words.begin()); it != m_Words.end(); it++) {
        qtb->insertPlainText(QStringLiteral(" "));
        qtb->insertHtml(*it);
    }
}

Glk::TextBufferDevice::TextBufferDevice(Glk::TextBufferWindow* win) : mp_TBWindow(win), m_CurrentHyperlink(0) {
    Q_ASSERT(mp_TBWindow);
}

qint64 Glk::TextBufferDevice::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 Glk::TextBufferDevice::writeData(const char* data, qint64 len) {
    Q_ASSERT(len % 4 == 0);

    qint64 ulen = len / 4;
    const glui32* udata = reinterpret_cast<const glui32*>(data);

    QString text = QString::fromUcs4(udata, ulen);
    QStringList blocks = text.split('\n');

    QStringList words = blocks.front().split(' ');

    if(m_Buffer.isEmpty()) {
        m_Buffer.append(Block());
        m_Buffer.back().appendWords(words, m_StyleString);
    } else {
        m_Buffer.back().appendWords(words, m_StyleString);
    }

    blocks.pop_front();

    for(const QString& b : blocks) {
        words.clear();
        words = b.split(' ');
        m_Buffer.append(Block());
        m_Buffer.back().appendWords(words, m_StyleString);
    }

    return len;
}

void Glk::TextBufferDevice::discard() {
    m_Buffer.clear();
}

void Glk::TextBufferDevice::flush() {
    if(m_Buffer.isEmpty())
        return;
    
    mp_TBWindow->mp_Text->moveCursor(QTextCursor::End);
    
    if(m_CurrentHyperlink != 0)
        m_Buffer.back().insertCloseHyperlinkTag();

    m_Buffer.front().writeToBrowser(mp_TBWindow->mp_Text);
    m_Buffer.pop_front();

    for(const Block& b : m_Buffer) {
        mp_TBWindow->mp_Text->insertPlainText(QStringLiteral("\n"));
        b.writeToBrowser(mp_TBWindow->mp_Text);
    }

    m_Buffer.clear();

    emit textChanged();
    
    if(m_CurrentHyperlink != 0) {
        m_Buffer.append(Block());
        m_Buffer.back().insertOpenHyperlinkTag(m_CurrentHyperlink);
    }
}

void Glk::TextBufferDevice::onHyperlinkPushed(glui32 linkval) {
    if(m_CurrentHyperlink == linkval)
        return;

    if(linkval == 0) {
        Q_ASSERT_X(!m_Buffer.isEmpty(), "Glk::TextBufferDevice::onHyperlinkPushed", "if hyperlink tag is till open when flushing, we need to ensure a new opening tag is appended");
        m_Buffer.back().insertCloseHyperlinkTag();
    } else {
        if(m_CurrentHyperlink != 0) {
            Q_ASSERT_X(!m_Buffer.isEmpty(), "Glk::TextBufferDevice::onHyperlinkPushed", "if hyperlink tag is till open when flushing, we need to ensure a new opening tag is appended");
            m_Buffer.back().insertCloseHyperlinkTag();
        }
        
        if(m_Buffer.isEmpty())
            m_Buffer.append(Block());
        
        m_Buffer.back().insertOpenHyperlinkTag(linkval);
    }

    m_CurrentHyperlink = linkval;
}

void Glk::TextBufferDevice::onWindowStyleChanged(const QString& newStyleString) {
    m_StyleString = newStyleString;
}

Glk::TextBufferWindow::TextBufferWindow(glui32 rock_) : Window(new TextBufferDevice(this), rock_, true, true, false, true), mp_Text(), m_Styles(QGlk::getMainWindow().textBufferStyleManager()), m_CurrentStyleType(Glk::Style::Normal), m_PreviousStyleType(Glk::Style::Normal) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    QLayout* lay = new QGridLayout(this);
    lay->setMargin(0);

    mp_Text = new QTextBrowser(this);
//     mp_Text = new QLabel(this);
//     mp_Text->setAlignment(Qt::AlignBottom);
    mp_Text->setReadOnly(true);
    mp_Text->setOpenExternalLinks(false);
    mp_Text->setOpenLinks(false);

    lay->addWidget(mp_Text);

    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::characterInputRequested,
        this, &Glk::TextBufferWindow::onCharacterInputRequested);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::characterInputRequestEnded,
        this, &Glk::TextBufferWindow::onCharacterInputRequestEnded);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputRequested,
        this, &Glk::TextBufferWindow::onLineInputRequested);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputRequestEnded,
        this, &Glk::TextBufferWindow::onLineInputRequestEnded);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputCharacterEntered,
        this, &Glk::TextBufferWindow::onCharacterInput);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputSpecialCharacterEntered,
        this, &Glk::TextBufferWindow::onSpecialCharacterInput);

    connect(
        mp_Text, &QTextBrowser::anchorClicked,
        this, &Glk::TextBufferWindow::onHyperlinkClicked);
    
    connect(
        windowStream(), &Glk::WindowStream::hyperlinkPushed,
        ioDevice(), &Glk::TextBufferDevice::onHyperlinkPushed);

    connect(
        ioDevice(), &Glk::TextBufferDevice::textChanged,
        this, &Glk::TextBufferWindow::onTextChanged);

    connect(
        &QGlk::getMainWindow(), &QGlk::poll,
        ioDevice(), &Glk::TextBufferDevice::flush);

    ioDevice()->onWindowStyleChanged(m_Styles[m_CurrentStyleType].styleString());
}

void Glk::TextBufferWindow::setStyle(Glk::Style::Type style) {
    qDebug() << "Changed style from" << m_CurrentStyleType << "to" << style;

    m_PreviousStyleType = m_CurrentStyleType;
    m_CurrentStyleType = style;
    ioDevice()->onWindowStyleChanged(m_Styles[m_CurrentStyleType].styleString());
}

void Glk::TextBufferWindow::clearWindow() {
    ioDevice()->discard();
    mp_Text->clear();
}

void Glk::TextBufferWindow::onHyperlinkClicked(const QUrl& link) {
    qDebug() << link;
    
    QString linkvalStr = link.toString().mid(1);
    glui32 linkval = linkvalStr.toUInt();
    
    Glk::postTaskToGlkThread([=]() {
        hyperlinkInputProvider()->handleHyperlinkClicked(linkval);
    });
}

void Glk::TextBufferWindow::onTextChanged() {
    mp_Text->verticalScrollBar()->setValue(mp_Text->verticalScrollBar()->maximum());
}

void Glk::TextBufferWindow::resizeEvent(QResizeEvent* ev) {
    QWidget::resizeEvent(ev);

//      mp_Text->verticalScrollBar()->setValue(mp_Text->verticalScrollBar()->maximum());
}

QSize Glk::TextBufferWindow::pixelsToUnits(const QSize& pixels) const {
    int hmargins = contentsMargins().left() + contentsMargins().right() + mp_Text->contentsMargins().left() + mp_Text->contentsMargins().right();
    int vmargins = contentsMargins().top() + contentsMargins().bottom() + mp_Text->contentsMargins().top() + mp_Text->contentsMargins().bottom();

    QSize u(std::clamp(pixels.width() - hmargins, 0, size().width() - hmargins), std::clamp(pixels.height() - vmargins, 0, size().height() - vmargins));

    u.setWidth(u.width() / mp_Text->fontMetrics().horizontalAdvance('0'));
    u.setHeight(u.height() / mp_Text->fontMetrics().height());

    return u;
}

QSize Glk::TextBufferWindow::unitsToPixels(const QSize& units) const {
    int hmargins = contentsMargins().left() + contentsMargins().right() + mp_Text->contentsMargins().left() + mp_Text->contentsMargins().right();
    int vmargins = contentsMargins().top() + contentsMargins().bottom() + mp_Text->contentsMargins().top() + mp_Text->contentsMargins().bottom();

    QSize u(hmargins + units.width()*mp_Text->fontMetrics().horizontalAdvance('0'), vmargins + units.height()*mp_Text->fontMetrics().height());

    return u;
}

void Glk::TextBufferWindow::onCharacterInputRequested() {
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
    ioDevice()->flush();
}

void Glk::TextBufferWindow::onCharacterInputRequestEnded(bool cancelled) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ioDevice()->flush();
}

void Glk::TextBufferWindow::onLineInputRequested() {
    setStyle(Glk::Style::Input);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
    ioDevice()->flush();
}

void Glk::TextBufferWindow::onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode) {
    if(!keyboardInputProvider()->echoesLine()) {
        for(glui32 ii = 0; ii < len; ii++) // equality because of newline at end
            mp_Text->textCursor().deletePreviousChar(); // TODO more efficient?
    } else {
        windowStream()->writeUnicodeChar('\n');

        if(windowStream()->echoStream()) {
            if(unicode)
                windowStream()->echoStream()->writeUnicodeBuffer(static_cast<glui32*>(buf), len);
            else
                windowStream()->echoStream()->writeBuffer(static_cast<char*>(buf), len);

            windowStream()->echoStream()->writeChar('\n');
        }
    }

    setStyle(m_PreviousStyleType);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    ioDevice()->flush();
}

void Glk::TextBufferWindow::onCharacterInput(glui32 ch) {
    windowStream()->writeUnicodeChar(ch);
    ioDevice()->flush();
}

void Glk::TextBufferWindow::onSpecialCharacterInput(glui32 kc) {
    switch(kc) {
        case keycode_Delete:
            mp_Text->textCursor().deletePreviousChar();
            break;

        case keycode_Return:
            Q_ASSERT_X(1, "handling line input special character", "this code shouldn't run");
            break;
//         case keycode_Left:
//             mp_Text->textCursor().movePosition(QTextCursor::PreviousCharacter);
//             break;
//         case keycode_Right:
//             mp_Text->textCursor().movePosition(QTextCursor::NextCharacter);
//             break;
    }

    ioDevice()->flush();
}

#include "moc_textbufferwindow.cpp"
