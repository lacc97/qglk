#include "textgridwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QFontDatabase>
#include <QPainter>
#include <QResizeEvent>

#include "qglk.hpp"

Glk::TextGridDevice::TextGridDevice(Glk::TextGridWindow* win) : mp_TGWindow(win) {}

qint64 Glk::TextGridDevice::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 Glk::TextGridDevice::writeData(const char* data, qint64 len) {
    assert(len % 4 == 0);

    const glui32* udata = reinterpret_cast<const glui32*>(data);
    qint64 ulen = len / 4;

    qint64 wcount;

    for(wcount = 0; wcount < ulen; wcount++) {
        if(!mp_TGWindow->writeChar(qFromBigEndian(udata[wcount])))
            break;
    }

    return wcount * 4;
}

Glk::TextGridWindow::TextGridWindow(glui32 rock_) : Window(new TextGridDevice(this), rock_, true, true), m_CharArray(), m_Cursor(0, 0) {
    m_CharArray.resize(1);
    m_CharArray[0].resize(1);

    setFocusPolicy(Qt::StrongFocus);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::characterInputRequested,
        this, &Glk::TextGridWindow::onCharacterInputRequested);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::characterInputRequestEnded,
        this, &Glk::TextGridWindow::onCharacterInputRequestEnded);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputRequested,
        this, &Glk::TextGridWindow::onLineInputRequested);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputRequestEnded,
        this, &Glk::TextGridWindow::onLineInputRequestEnded);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputCharacterEntered,
        this, &Glk::TextGridWindow::onCharacterInput);
    connect(
        keyboardInputProvider(), &Glk::KeyboardInputProvider::lineInputSpecialCharacterEntered,
        this, &Glk::TextGridWindow::onSpecialCharacterInput);

    connect(
        &QGlk::getMainWindow(), &QGlk::poll,
        this, qOverload<>(&Glk::TextGridWindow::update));
}

void Glk::TextGridWindow::clearWindow() {
    for(QVector<glui32>& column : m_CharArray) {
        for(glui32& ch : column) {
            ch = 0;
        }
    }

    m_Cursor = QPoint(0, 0);
}

void Glk::TextGridWindow::paintEvent(QPaintEvent* event) {
    QRect region(pixelsToUnits(event->region().boundingRect().topLeft()), pixelsToUnits(event->region().boundingRect().size()));
    QPainter painter(this);

    int vertiadv = fontMetrics().height();
    int horizadv = fontMetrics().horizontalAdvance('m');
    int baseline = fontMetrics().ascent();

    int regx2 = std::min(region.right(), m_CharArray.size() - 1);
    int regy2 = std::min(region.bottom(), m_CharArray[0].size() - 1);

    for(int xx = region.left(); xx <= regx2; xx++) {
        for(int yy = region.top(); yy <= regy2; yy++) {
            painter.drawText(contentsMargins().left() + horizadv * xx, contentsMargins().top() + vertiadv * yy + baseline, QString::fromUcs4(&m_CharArray[xx][yy], 1));
        }
    }
}

void Glk::TextGridWindow::resizeEvent(QResizeEvent* ev) {
    QSize olds = pixelsToUnits(ev->oldSize());
    QSize news = pixelsToUnits(ev->size());

    m_CharArray.resize(std::max(1, news.width()));

    for(QVector<glui32>& column : m_CharArray)
        column.resize(std::max(1, news.height()));

    if(news.width() > olds.width()) {
        for(int xx = olds.width(); xx < news.width(); xx++) {
            for(glui32& ch : m_CharArray[xx]) {
                ch = 0;
            }
        }
    }

    if(news.height() > olds.height()) {
        for(QVector<glui32>& column : m_CharArray) {
            for(int yy = olds.height(); yy < news.height(); yy++) {
                column[yy] = 0;
            }
        }
    }

    // TODO modify content margins to center everything

    // TODO reset cursor to origin?
}

QSize Glk::TextGridWindow::pixelsToUnits(const QSize& pixels) const {
    int hmargins = contentsMargins().left() + contentsMargins().right();
    int vmargins = contentsMargins().top() + contentsMargins().bottom();

    QSize u(std::clamp(pixels.width() - hmargins, 0, size().width() - hmargins), std::clamp(pixels.height() - vmargins, 0, size().height() - vmargins));

    u.setWidth(u.width() / fontMetrics().horizontalAdvance('0'));
    u.setHeight(u.height() / fontMetrics().height());

    return u;
}

QPoint Glk::TextGridWindow::pixelsToUnits(QPoint pixels) const {
    int hmargins = contentsMargins().left() + contentsMargins().right();
    int vmargins = contentsMargins().top() + contentsMargins().bottom();

    QPoint u(std::clamp(pixels.x() - hmargins, 0, size().width() - hmargins), std::clamp(pixels.y() - vmargins, 0, size().height() - vmargins));

    u.setX(u.x() / fontMetrics().horizontalAdvance('0'));
    u.setY(u.x() / fontMetrics().height());

    return u;
}

QSize Glk::TextGridWindow::unitsToPixels(const QSize& units) const {
    int pw = (contentsMargins().left() + contentsMargins().right() + units.width() * fontMetrics().horizontalAdvance('0'));
    int ph = (contentsMargins().top() + contentsMargins().bottom() + units.height() * fontMetrics().height());

    return QSize(pw, ph);
}

bool Glk::TextGridWindow::writeChar(glui32 ch) {
    QSize ws(m_CharArray.size(), m_CharArray[0].size());

    if(m_Cursor.x() >= ws.width() || m_Cursor.y() >= ws.height())
        return false;

    if(ch == '\n') {
        m_Cursor = QPoint(0, m_Cursor.y() + 1);
        return true;
    }

    m_CharArray[m_Cursor.x()][m_Cursor.y()] = ch;
    m_Cursor += QPoint(1, 0);

    if(m_Cursor.x() == ws.width()) {
        m_Cursor.setX(0);
        m_Cursor += QPoint(0, 1);
    }

    return true;
}

bool Glk::TextGridWindow::deletePreviousChar() {
    if(m_Cursor.x() == 0 && m_Cursor.y() == 0)
        return false;

    if(m_Cursor.x() == 0)
        m_Cursor = QPoint(m_CharArray[m_Cursor.y() - 1].size() - 1, m_Cursor.y() - 1);
    else
        m_Cursor -= QPoint(1, 0);

    m_CharArray[m_Cursor.x()][m_Cursor.y()] = 0;

    return true;
}

void Glk::TextGridWindow::onCharacterInputRequested() {
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
}

void Glk::TextGridWindow::onCharacterInputRequestEnded(bool cancelled) {
    setFocusPolicy(Qt::FocusPolicy::NoFocus);
}

void Glk::TextGridWindow::onLineInputRequested() {
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);
    setFocus();
}

void Glk::TextGridWindow::onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode) {
    if(cancelled) {
//         for(glui32 ii = 0; ii < len; ii++)
//             mp_Text->textCursor().deletePreviousChar(); // TODO more efficient?
    } else {
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

void Glk::TextGridWindow::onCharacterInput(glui32 ch) {
    writeChar(ch);
}

void Glk::TextGridWindow::onSpecialCharacterInput(glui32 kc) {
    switch(kc) {
        case keycode_Delete:
            deletePreviousChar();
            break;

        case keycode_Return:
            writeChar('\n');
            break;
//         case keycode_Left:
//             mp_Text->textCursor().movePosition(QTextCursor::PreviousCharacter);
//             break;
//         case keycode_Right:
//             mp_Text->textCursor().movePosition(QTextCursor::NextCharacter);
//             break;
    }
}

#include "moc_textgridwindow.cpp"

