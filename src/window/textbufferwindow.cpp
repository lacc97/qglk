#include "textbufferwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollBar>

#include "qglk.hpp"

Glk::TextBufferDevice::TextBufferDevice(Glk::TextBufferWindow* win) : mp_TBWindow(win) {
    Q_ASSERT(mp_TBWindow);
}

qint64 Glk::TextBufferDevice::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 Glk::TextBufferDevice::writeData(const char* data, qint64 len) {
    Q_ASSERT(len % 4 == 0);

    qint64 ulen = len / 4;
    glui32* udata = new glui32[ulen];

    for(glui32 ii = 0; ii < ulen; ii++)
        udata[ii] = qFromBigEndian(reinterpret_cast<const glui32*>(data)[ii]);
    
    QString text = QString::fromUcs4(udata, ulen);
    QStringList blocks = text.split('\n');
    
    insertText(blocks.front());
    blocks.pop_front();
    
    for(const QString& block : blocks) {
        mp_TBWindow->mp_Text->insertPlainText(QStringLiteral("\n"));
        insertText(block);
    }

//     qDebug() << mp_TBWindow->mp_Text->document()->defaultStyleSheet();
//     mp_TBWindow->mp_Text->insertPlainText(QString::fromUcs4(udata, ulen));
//     qDebug() << mp_TBWindow->mp_Text->toHtml();

    emit textChanged();

    delete[] udata;

    return len;
}

void Glk::TextBufferDevice::onWindowStyleChanged(const QString& newStyleString) {
//     cleanClosingSpanTag();
//     mp_TBWindow->mp_Text->insertPlainText(QStringLiteral("<span style=\"%1\"></span>").arg(newStyle.styleString()));
// //     mp_TBWindow->mp_Text->document()->setDefaultStyleSheet(QStringLiteral("p{%1}").arg(newStyle.styleString()));
    m_StyleString = newStyleString;
}

void Glk::TextBufferDevice::cleanClosingSpanTag() {
    mp_TBWindow->mp_Text->textCursor().movePosition(QTextCursor::PreviousCharacter, QTextCursor::MoveAnchor, 7);
//     for(size_t ii = 0; ii < 7; ii++) mp_TBWindow->mp_Text->textCursor().deletePreviousChar();
}

void Glk::TextBufferDevice::insertText(const QString& str) {
    QString textString = str;
    
    while(textString[0] == ' ') {
        mp_TBWindow->mp_Text->insertPlainText(QStringLiteral(" "));
        textString.remove(0,1);
    }
    
    if(!textString.isEmpty())
        mp_TBWindow->mp_Text->insertHtml(QStringLiteral("<span style=\"%1\">%2</span>").arg(m_StyleString).arg(str));
//     qDebug() << QStringLiteral("<span style=\"%1\">%2</span>").arg(m_StyleString).arg(str);
}

Glk::TextBufferWindow::TextBufferWindow(glui32 rock_) : Window(new TextBufferDevice(this), rock_, true, true), mp_Text(), m_Styles(QGlk::getMainWindow().textBufferStyleManager()), m_CurrentStyleType(Glk::Style::Normal), m_PreviousStyleType(Glk::Style::Normal) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);

    QLayout* lay = new QGridLayout(this);
    lay->setMargin(0);

    mp_Text = new QTextBrowser(this);
//     mp_Text = new QLabel(this);
//     mp_Text->setAlignment(Qt::AlignBottom);
    mp_Text->setReadOnly(true);
    mp_Text->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);

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
        ioDevice(), &Glk::TextBufferDevice::textChanged,
        this, &Glk::TextBufferWindow::onTextChanged);
//     connect(
//         this, &Glk::TextBufferWindow::styleChanged,
//         ioDevice(), &Glk::TextBufferDevice::onWindowStyleChanged);
    
    ioDevice()->onWindowStyleChanged(m_Styles[m_CurrentStyleType].styleString());
}

void Glk::TextBufferWindow::setStyle(Glk::Style::Type style) {
    qDebug() << "Changed style from" << m_CurrentStyleType << "to" << style; 
    
    m_PreviousStyleType = m_CurrentStyleType;
    m_CurrentStyleType = style;
    ioDevice()->onWindowStyleChanged(m_Styles[m_CurrentStyleType].styleString());
}

void Glk::TextBufferWindow::clearWindow() {
    mp_Text->clear();
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
}

void Glk::TextBufferWindow::onCharacterInputRequestEnded(bool cancelled) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

void Glk::TextBufferWindow::onLineInputRequested() {
    setStyle(Glk::Style::Input);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
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
}

void Glk::TextBufferWindow::onCharacterInput(glui32 ch) {
    windowStream()->writeUnicodeChar(ch);
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
}

#include "moc_textbufferwindow.cpp"
