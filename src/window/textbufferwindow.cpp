#include "textbufferwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollBar>

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

    mp_TBWindow->mp_Text->insertPlainText(QString::fromUcs4(udata, ulen));
//     mp_TBWindow->mp_Text->setText(mp_TBWindow->mp_Text->text() + QString::fromUcs4(udata, ulen));

    delete[] udata;
    
    return len;
}

Glk::TextBufferWindow::TextBufferWindow(glui32 rock_) : Window(new TextBufferDevice(this), rock_, true, true), mp_Text() {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
    QObject::connect(keyboardInputProvider(), SIGNAL(characterInputRequested()), this, SLOT(onCharacterInputRequested()));
    QObject::connect(keyboardInputProvider(), SIGNAL(characterInputRequestEnded(bool)), this, SLOT(onCharacterInputRequestEnded(bool)));
    QObject::connect(keyboardInputProvider(), SIGNAL(lineInputRequested()), this, SLOT(onLineInputRequested()));
    QObject::connect(keyboardInputProvider(), SIGNAL(lineInputRequestEnded(bool, void*, glui32, bool)), this, SLOT(onLineInputRequestEnded(bool, void*, glui32, bool)));
    QObject::connect(keyboardInputProvider(), SIGNAL(lineInputCharacterEntered(glui32)), this, SLOT(onCharacterInput(glui32)));
    QObject::connect(keyboardInputProvider(), SIGNAL(lineInputSpecialCharacterEntered(glui32)), this, SLOT(onSpecialCharacterInput(glui32)));

    QLayout* lay = new QGridLayout(this);
    lay->setMargin(0);

    mp_Text = new QTextBrowser(this);
//     mp_Text = new QLabel(this);
//     mp_Text->setAlignment(Qt::AlignBottom);
    mp_Text->setReadOnly(true);
    mp_Text->setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::LinksAccessibleByKeyboard);
    QObject::connect(mp_Text, SIGNAL(textChanged()), this, SLOT(onTextChanged()));

    lay->addWidget(mp_Text);
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
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
}

void Glk::TextBufferWindow::onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode) {
    if(!keyboardInputProvider()->echoesLine()) {
        for(glui32 ii = 0; ii < len; ii++) // equality because of newline at end
            mp_Text->textCursor().deletePreviousChar(); // TODO more efficient?
    } else {
        mp_Text->insertPlainText("\n");
        
        if(windowStream()->echoStream()) {
            if(unicode)
                windowStream()->echoStream()->writeUnicodeBuffer(static_cast<glui32*>(buf), len);
            else
                windowStream()->echoStream()->writeBuffer(static_cast<char*>(buf), len);

            windowStream()->echoStream()->writeChar('\n');
        }
    }

    setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

void Glk::TextBufferWindow::onCharacterInput(glui32 ch) {
    mp_Text->insertPlainText(QString::fromUcs4(&ch, 1));
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
