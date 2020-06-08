#include "textbufferwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollBar>

#include "qglk.hpp"

#include "log/log.hpp"


Glk::TextBufferDevice::TextBufferDevice(Glk::TextBufferWindow* win)
    : WindowDevice{win} {}

qint64 Glk::TextBufferDevice::readData(char* data, qint64 maxlen) {
    return 0;
}

qint64 Glk::TextBufferDevice::writeData(const char* data, qint64 len) {
    assert(len % 4 == 0);

    window<TextBufferWindow>()->writeString(QString::fromUcs4(reinterpret_cast<const glui32*>(data), len / 4));

    return len;
}


Glk::TextBufferWindow::TextBufferWindow(Glk::TextBufferWindowController* winController,
                                        Glk::PairWindow* winParent, glui32 winRock)
    : Window(Type::TextBuffer, winController, new TextBufferDevice{this}, winParent, winRock),
      m_Images{},
      m_Styles{QGlk::getMainWindow().textBufferStyleManager()},
      m_CurrentStyleType{Style::Normal},
      m_CurrentBlockFormat{m_Styles[m_CurrentStyleType].blockFormat()},
      m_CurrentCharFormat{m_Styles[m_CurrentStyleType].charFormat()},
      m_NonHyperlinkCharFormat{m_CurrentCharFormat},
      m_CurrentHyperlink{0} {
    assert(onGlkThread());
}

void Glk::TextBufferWindow::clearWindow() {
    assert(onGlkThread());

    m_Images.clear();

    controller<TextBufferWindowController>()->clearDocument();
}

bool Glk::TextBufferWindow::drawImage(const QImage& img, glsi32 param1, glsi32 param2, QSize imgSize) {
    assert(onGlkThread());

    size_t imgNum = m_Images.size();
    m_Images.push_back(img);

    QString altAttrib = QStringLiteral("alt=\"%1\"").arg(imgNum);
    QString sizeAttrib = QStringLiteral("width=\"%1\" height=\"%2\"").arg(imgSize.width()).arg(imgSize.height());
    QString alignAttrib;

    switch(param1) {
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

    controller<TextBufferWindowController>()->writeHTML(
        QStringLiteral("<img src=\"%1\" %2 %3 %4 />").arg(imgNum).arg(altAttrib).arg(sizeAttrib).arg(alignAttrib));

    return true;
}

void Glk::TextBufferWindow::flowBreak() {
    assert(onGlkThread());

    controller<TextBufferWindowController>()->flowBreak();
}

void Glk::TextBufferWindow::pushHyperlink(glui32 linkValue) {
    assert(onGlkThread());

    m_CurrentHyperlink = linkValue;
    m_CurrentCharFormat = m_NonHyperlinkCharFormat;

    if(m_CurrentHyperlink != 0) {
        m_CurrentCharFormat.setAnchor(true);
        m_CurrentCharFormat.setAnchorHref(QStringLiteral("%1").arg(m_CurrentHyperlink));
        m_CurrentCharFormat.setForeground(Qt::blue);
        m_CurrentCharFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
    }
}

void Glk::TextBufferWindow::pushStyle(Glk::Style::Type style) {
    assert(onGlkThread());

    m_CurrentStyleType = style;
    m_CurrentBlockFormat = m_Styles[m_CurrentStyleType].blockFormat();
    m_CurrentCharFormat = m_Styles[m_CurrentStyleType].charFormat();
    m_NonHyperlinkCharFormat = m_CurrentCharFormat;

    pushHyperlink(m_CurrentHyperlink);
}

void Glk::TextBufferWindow::writeString(const QString& str) {
    controller<TextBufferWindowController>()->writeString(str, m_CurrentCharFormat, m_CurrentBlockFormat);
}
