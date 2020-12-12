#include "textbufferwindow.hpp"

#include <algorithm>

#include <QtEndian>

#include <QGridLayout>
#include <QResizeEvent>
#include <QScrollBar>

#include "qglk.hpp"

#include "log/log.hpp"


Glk::TextBufferBuf::TextBufferBuf(Glk::TextBufferWindow* win)
    : WindowBuf{win} {}

std::streamsize Glk::TextBufferBuf::xsputn(const char_type* s, std::streamsize count) {
    assert(count % 4 == 0);

    window<TextBufferWindow>()->writeString(QString::fromUcs4((const char32_t*)s, count / sizeof(glui32)));

    return count;
}


Glk::TextBufferWindow::TextBufferWindow(Glk::TextBufferWindowController* winController, Glk::PairWindow* winParent, glui32 winRock)
    : Window(Type::TextBuffer, winController, std::make_unique<TextBufferBuf>(this), winParent, winRock),
      m_Styles{QGlk::getMainWindow().textBufferStyleManager()} {
    assert(onGlkThread());
}

void Glk::TextBufferWindow::clearWindow() {
    assert(onGlkThread());

    controller<TextBufferWindowController>()->pushCommand(TextBufferCommand::Clear{});
}

bool Glk::TextBufferWindow::drawImage(glui32 img, glsi32 param1, glsi32 param2, QSize size) {
    assert(onGlkThread());

    std::u16string_view style;
    switch(param1) {
        case imagealign_InlineUp:
            style = u"vertical-align: top";
            break;

        case imagealign_InlineCenter:
            style = u"vertical-align: middle";
            break;

        case imagealign_InlineDown:
            style = u"vertical-align: bottom";
            break;

        case imagealign_MarginLeft:
            style = u"float: left";
            break;

        case imagealign_MarginRight:
            style = u"float: right";
            break;
    }

    controller<TextBufferWindowController>()->pushCommand(TextBufferCommand::WriteImage{img, size, style});

    return true;
}

void Glk::TextBufferWindow::flowBreak() {
    assert(onGlkThread());

    controller<TextBufferWindowController>()->pushCommand(TextBufferCommand::FlowBreak{});
}

void Glk::TextBufferWindow::pushHyperlink(glui32 linkValue) {
    assert(onGlkThread());

    controller<TextBufferWindowController>()->pushCommand(TextBufferCommand::HyperlinkPush{linkValue});
}

void Glk::TextBufferWindow::pushStyle(Glk::Style::Type style) {
    assert(onGlkThread());

    controller<TextBufferWindowController>()->pushCommand(TextBufferCommand::StylePush{m_Styles[style]});
}

void Glk::TextBufferWindow::writeString(QString str) {
    controller<TextBufferWindowController>()->pushCommand(TextBufferCommand::WriteText{std::move(str)});
}