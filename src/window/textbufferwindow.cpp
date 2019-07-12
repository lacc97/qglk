#include "textbufferwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollBar>

#include "qglk.hpp"

#include "log/log.hpp"

Glk::TextBufferDevice::FormattedText::FormattedText(const QString& qstr, const QTextBlockFormat& blkfmt, const QTextCharFormat& chfmt) : m_Text(qstr), m_BlockFormat(blkfmt), m_CharFormat(chfmt) {}

void Glk::TextBufferDevice::FormattedText::writeToCursor(QTextCursor& cursor) const {
    cursor.setBlockFormat(m_BlockFormat);
    cursor.setCharFormat(m_CharFormat);

    cursor.insertText(m_Text);
}

Glk::TextBufferDevice::TextBufferDevice(Glk::TextBufferWindow* win) : mp_TBWindow(win), m_CurrentHyperlink(0) {
    Q_ASSERT(mp_TBWindow);
}

void Glk::TextBufferDevice::drawImage(int imgnum, glsi32 alignment, glui32 w, glui32 h) {
    QString altAttrib = QStringLiteral("alt=\"%1\"").arg(imgnum);
    QString sizeAttrib = QStringLiteral("width=\"%1\" height=\"%2\"").arg(w).arg(h);
    QString alignAttrib;

    switch(alignment) {
        case imagealign_InlineUp:
            alignAttrib = QStringLiteral("style=\"vertical-align: %1;\"").arg(QStringLiteral("top"));
            break;

        case imagealign_InlineCenter:
            alignAttrib = QStringLiteral("style=\"vertical-align: %1;\"").arg(QStringLiteral("middle"));
            break;

        case imagealign_InlineDown:
            alignAttrib = QStringLiteral("style=\"vertical-align: %1;\"").arg(QStringLiteral("bottom"));
            break;

        case imagealign_MarginLeft:
            alignAttrib = QStringLiteral("style=\"float: %1;\"").arg(QStringLiteral("left"));
            break;

        case imagealign_MarginRight:
            alignAttrib = QStringLiteral("style=\"float: %1;\"").arg(QStringLiteral("right"));
            break;
    }

    flush();

    mp_TBWindow->m_EditingCursor.insertHtml(QStringLiteral("<img src=\"%1\" %2 %3 %4 />").arg(imgnum).arg(altAttrib).arg(sizeAttrib).arg(alignAttrib));
}

qint64 Glk::TextBufferDevice::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 Glk::TextBufferDevice::writeData(const char* data, qint64 len) {
    Q_ASSERT(len % 4 == 0);

    qint64 ulen = len / 4;
    auto udata = reinterpret_cast<const glui32*>(data);

    QString text = QString::fromUcs4(udata, ulen);
//     QStringList blocks = text.split('\n');
//
//     QStringList words = blocks.front().split(' ');

    if(m_Buffer.isEmpty())
        m_Buffer.append(FormattedText(QString(), m_CurrentBlockFormat, m_CurrentCharFormat));

    m_Buffer.back().appendText(text);

    return len;
}

void Glk::TextBufferDevice::discard() {
    m_Buffer.clear();
}

void Glk::TextBufferDevice::flush() {
    if(m_Buffer.isEmpty())
        return;

    for(const FormattedText& text : qAsConst(m_Buffer))
        text.writeToCursor(mp_TBWindow->m_EditingCursor);

    m_Buffer.clear();

    if(m_CurrentHyperlink != 0) {
        QTextCharFormat chfmt(m_CurrentCharFormat);
        chfmt.setAnchor(true);
        chfmt.setAnchorHref(QStringLiteral("%1").arg(m_CurrentHyperlink));

        m_Buffer.append(FormattedText(QString(), m_CurrentBlockFormat, chfmt));
    }

    emit textChanged();
}

void Glk::TextBufferDevice::onHyperlinkPushed(glui32 linkval) {
    if(m_CurrentHyperlink == linkval)
        return;

    m_CurrentHyperlink = linkval;

    if(m_CurrentHyperlink == 0) {
        m_Buffer.append(FormattedText(QString(), m_CurrentBlockFormat, m_CurrentCharFormat));
    } else {
        QTextCharFormat chfmt(m_CurrentCharFormat);
        chfmt.setAnchor(true);
        chfmt.setAnchorHref(QStringLiteral("%1").arg(m_CurrentHyperlink));
        chfmt.setForeground(Qt::blue);
        chfmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);

        m_Buffer.append(FormattedText(QString(), m_CurrentBlockFormat, chfmt));
    }
}

void Glk::TextBufferDevice::onWindowStyleChanged(const QTextBlockFormat& blkfmt, const QTextCharFormat& chfmt) {
    m_CurrentBlockFormat = blkfmt;
    m_CurrentCharFormat = chfmt;

    m_Buffer.append(FormattedText(QString(), m_CurrentBlockFormat, m_CurrentCharFormat));
}

Glk::TextBufferWindow::History::History() : m_History(), m_Iterator(m_History.begin()) {
}

void Glk::TextBufferWindow::History::push(const QString& newcmd) {
    if(m_History.size() > MAX_SIZE)
        m_History.takeLast();

    m_History.prepend(newcmd);
}

void Glk::TextBufferWindow::History::resetIterator() {
    m_Iterator = m_History.begin();
}

const QString Glk::TextBufferWindow::History::next() {
    if(m_History.size() == 0)
        return QString();

    if(m_Iterator == m_History.end())
        return m_History.back();

    return *(m_Iterator++);
}

const QString Glk::TextBufferWindow::History::previous() {
    if(m_History.size() == 0)
        return QString();

    if(m_Iterator == m_History.begin())
        return QString();

    auto it = --m_Iterator;

    if(it == m_History.begin())
        return QString();

    return *(--it);
}

int Glk::TextBufferBrowser::numImages() const {
    return m_ImageList.size();
}

void Glk::TextBufferBrowser::addImage(const QImage& im) {
    m_ImageList.append(im);
}

void Glk::TextBufferBrowser::clearImages() {
    m_ImageList.clear();
}

QVariant Glk::TextBufferBrowser::loadResource(int type, const QUrl& name) {
    if(type == QTextDocument::ImageResource) {
        int imgIndex = name.toString().toUInt();

        if(imgIndex < m_ImageList.size())
            return QVariant::fromValue(m_ImageList[imgIndex]);
    }

    return QTextBrowser::loadResource(type, name);
}

void Glk::TextBufferBrowser::keyPressEvent(QKeyEvent* event) {
    switch(event->key()) {
        case Qt::Key_Down:   // disable
        case Qt::Key_Left:   // scrolling
        case Qt::Key_Right:  // with arrow
        case Qt::Key_Up:     // keys

        case Qt::Key_Space:  // fix space sometimes not recognised as input
            QWidget::keyPressEvent(event);
            break;

        default:
            QTextBrowser::keyPressEvent(event);
            break;
    }
}

Glk::TextBufferWindow::TextBufferWindow(glui32 rock_) : Window(new TextBufferDevice(this), rock_, true, true, false, true), mp_Text(), m_EditingCursor(), m_History(), m_Styles(QGlk::getMainWindow().textBufferStyleManager()), m_CurrentStyleType(Glk::Style::Normal), m_PreviousStyleType(Glk::Style::Normal) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    QLayout* lay = new QGridLayout(this);
    lay->setMargin(0);

    mp_Text = new TextBufferBrowser(this);
//     mp_Text = new QLabel(this);
//     mp_Text->setAlignment(Qt::AlignBottom);
    mp_Text->setReadOnly(true);
    mp_Text->setOpenExternalLinks(false);
    mp_Text->setOpenLinks(false);

    m_EditingCursor = mp_Text->textCursor();
    m_EditingCursor.movePosition(QTextCursor::End);

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

    ioDevice()->onWindowStyleChanged(m_Styles[m_CurrentStyleType].blockFormat(), m_Styles[m_CurrentStyleType].charFormat());
}

bool Glk::TextBufferWindow::drawImage(const QImage& im, glsi32 alignment, glui32 w, glui32 h) {
    int imageIndex = mp_Text->numImages();
    mp_Text->addImage(im);

    ioDevice()->drawImage(imageIndex, alignment, w, h);

    return true;
}

void Glk::TextBufferWindow::setStyle(Glk::Style::Type style) {
    log_debug() << "Changed style from " << m_CurrentStyleType << " to " << style;

    m_PreviousStyleType = m_CurrentStyleType;
    m_CurrentStyleType = style;
    ioDevice()->onWindowStyleChanged(m_Styles[m_CurrentStyleType].blockFormat(), m_Styles[m_CurrentStyleType].charFormat());
}

void Glk::TextBufferWindow::clearWindow() {
    ioDevice()->discard();
    mp_Text->clear();
    mp_Text->clearImages();
}

void Glk::TextBufferWindow::onHyperlinkClicked(const QUrl& link) {
    glui32 linkval = link.toString().toUInt();

    Glk::postTaskToGlkThread([ = ]() {
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
//    ioDevice()->flush();
}

void Glk::TextBufferWindow::onCharacterInputRequestEnded(bool cancelled) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
//    ioDevice()->flush();
}

void Glk::TextBufferWindow::onLineInputRequested() {
    setStyle(Glk::Style::Input);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
//    ioDevice()->flush();
}

void Glk::TextBufferWindow::onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode) {
    m_EditingCursor.movePosition(QTextCursor::End);

    if(!keyboardInputProvider()->echoesLine()) {
        for(glui32 ii = 0; ii < len; ii++) // equality because of newline at end
            m_EditingCursor.deletePreviousChar(); // TODO more efficient?
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

    if(unicode)
        m_History.push(QString::fromUcs4(static_cast<glui32*>(buf), len));
    else
        m_History.push(QString::fromLatin1(static_cast<char*>(buf), len));

    m_History.resetIterator();

    setStyle(m_PreviousStyleType);
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
//    ioDevice()->flush();
}

void Glk::TextBufferWindow::onCharacterInput(glui32 ch, bool doFlush) {
    windowStream()->writeUnicodeChar(ch);

    if(doFlush)
        ioDevice()->flush();
}

void Glk::TextBufferWindow::onSpecialCharacterInput(glui32 kc, bool doFlush) {
    switch(kc) {
        case keycode_Delete:
            m_EditingCursor.deletePreviousChar();
            break;

        case keycode_Down:
            keyboardInputProvider()->clearLineInputBuffer();
            keyboardInputProvider()->fillLineInputBuffer(m_History.previous());
            break;

        case keycode_Return:
            Q_ASSERT_X(1, "handling line input special character", "this code shouldn't run");
            break;

        case keycode_Left:
            m_EditingCursor.movePosition(QTextCursor::PreviousCharacter);
            break;

        case keycode_Right:
            m_EditingCursor.movePosition(QTextCursor::NextCharacter);
            break;

        case keycode_Up:
            keyboardInputProvider()->clearLineInputBuffer();
            keyboardInputProvider()->fillLineInputBuffer(m_History.next());
            break;
    }

    if(doFlush)
        ioDevice()->flush();
}

#include "moc_textbufferwindow.cpp"
