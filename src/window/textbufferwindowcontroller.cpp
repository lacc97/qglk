#include "textbufferwindowcontroller.hpp"

#include <QSemaphore>
#include <QThread>

#include "qglk.hpp"
#include "log/log.hpp"
#include "thread/taskrequest.hpp"

#include "textbufferwidget.hpp"
#include "textbufferwindow.hpp"

Glk::TextBufferWindowController::TextBufferWindowController(Glk::PairWindow* winParent, glui32 winRock)
    : WindowController(new TextBufferWindow(this, winParent, winRock), createWidget()) {
    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequestCancelled,
                     [this](const QString& text, bool lineEchoes) {
                         if(lineEchoes && !text.isEmpty()) {
                             Glk::postTaskToGlkThread([this, text = text + '\n']() {
                                 auto win = window<TextBufferWindow>();
                                 if(win) {
                                     win->pushStyle(Glk::Style::Type::Input);
                                     win->writeString(text);
                                 }
                             });
                         }

                         requestSynchronization();
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequestFulfilled,
                     [this](const QString& text, bool lineEchoes) {
                         if(lineEchoes && !text.isEmpty()) {
                             Glk::postTaskToGlkThread([this, text = text + '\n']() {
                                 auto win = window<TextBufferWindow>();
                                 if(win) {
                                     win->pushStyle(Glk::Style::Type::Input);
                                     win->writeString(text);
                                 }
                             });
                         }

                         requestSynchronization();
                     });
}

Glk::TextBufferWindowController::~TextBufferWindowController() = default;

void Glk::TextBufferWindowController::synchronize() {
    assert(onEventThread());

    if(!keyboardProvider()->lineInputRequest() || !keyboardProvider()->lineInputRequest()->isPending()) {
        synchronizeText();
        synchronizeInputStyle();
    }

    WindowController::synchronize();
}

QPoint Glk::TextBufferWindowController::glkPos(const QPoint& qtPos) const {
    spdlog::warn("Requesting glk position of Qt point ({}, {}) for window {}", qtPos.x(), qtPos.y(), wrap::ptr(window()));

    return qtPos;
}

QSize Glk::TextBufferWindowController::glkSize() const {
    QRect widgetBrowserFrameRect = widget<TextBufferWidget>()->browser()->frameRect();
    return {widgetBrowserFrameRect.width() / widget()->fontMetrics().horizontalAdvance('m'),
            widgetBrowserFrameRect.height() / widget()->fontMetrics().height()};
}

QSize Glk::TextBufferWindowController::toQtSize(const QSize& glk) const {
    int w = glk.width() * widget()->fontMetrics().horizontalAdvance('0');
    int h = glk.height() * widget()->fontMetrics().height();

    return {w + widget()->contentsMargins().left() + widget()->contentsMargins().right(),
            h + widget()->contentsMargins().top() + widget()->contentsMargins().bottom()};
}

bool Glk::TextBufferWindowController::supportsCharInput() const {
    return true;
}

bool Glk::TextBufferWindowController::supportsHyperlinkInput() const {
    return true;
}

bool Glk::TextBufferWindowController::supportsLineInput() const {
    return true;
}

void Glk::TextBufferWindowController::pushCommand(Glk::TextBufferWindowController::Command cmd) {
    std::visit([this](auto&& c) {
        using T = std::decay_t<decltype(c)>;

        if constexpr(std::is_same_v<T, TextBufferCommand::WriteText>) {
            if(!m_Commands.empty()) {
                auto merged = std::visit([&c](auto&& b) -> bool {
                    using U = std::decay_t<decltype(b)>;

                    if constexpr(std::is_same_v<U, TextBufferCommand::WriteText>) {
                        /* merge two consecutive WriteText commands together */
                        b.text.append(c.text);
                        return true;
                    } else {
                        return false;
                    }
                }, m_Commands.back());

                if(merged)
                    return;
            }
        }

        m_Commands.emplace_back(std::move(c));
    }, cmd);

    requestSynchronization();
}

QWidget* Glk::TextBufferWindowController::createWidget() {
    QWidget* w = nullptr;

    Glk::sendTaskToEventThread([&w]() {
        w = new TextBufferWidget;
        w->hide();
    });

    return w;
}

void Glk::TextBufferWindowController::synchronizeInputStyle() {
    Style inputStyle = window<TextBufferWindow>()->styles()[Style::Input];

    widget<TextBufferWidget>()->browser()->setInputBlockFormat(inputStyle.blockFormat());
    widget<TextBufferWidget>()->browser()->setInputCharFormat(inputStyle.charFormat());
}

void Glk::TextBufferWindowController::synchronizeText() {
    QTextDocument& doc = *widget<TextBufferWidget>()->browser()->document();
    QTextCursor cur{&doc};

    glui32 link = 0;
    Style style = window<TextBufferWindow>()->styles()[Style::Type::Normal];
    QTextCharFormat charFormat = style.charFormat();

    auto fn_push_link = [&cur, &link, &style, &charFormat](glui32 newLink) {
        link = newLink;
        charFormat = style.charFormat();

        if(link != 0) {
            charFormat.setAnchor(true);
            charFormat.setAnchorHref(QStringLiteral("%1").arg(link));
            charFormat.setForeground(Qt::blue);
            charFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);
        }

        cur.setCharFormat(charFormat);
    };
    auto fn_push_style = [&cur, &link, &style, &fn_push_link](Style newStyle) {
        style = std::move(newStyle);
        cur.setBlockFormat(style.blockFormat());
        fn_push_link(link);
    };

    cur.movePosition(QTextCursor::End);
    for(const auto& cmd : m_Commands) {
        std::visit([&doc, &cur, &fn_push_link, &fn_push_style](auto&& cmd) {
            using T = std::decay_t<decltype(cmd)>;

            if constexpr(std::is_same_v<T, TextBufferCommand::Clear>) {
                doc.clear();
                cur.movePosition(QTextCursor::Start);
            }
            if constexpr(std::is_same_v<T, TextBufferCommand::FlowBreak>) {
                cur.insertBlock();
            }
            if constexpr(std::is_same_v<T, TextBufferCommand::HyperlinkPush>) {
                fn_push_link(cmd.link);
            }
            if constexpr(std::is_same_v<T, TextBufferCommand::StylePush>) {
                fn_push_style(std::move(cmd.style));
            }
            if constexpr(std::is_same_v<T, TextBufferCommand::WriteImage>) {
                std::string_view fmtStr;
                if(cmd.size.isEmpty())
                    fmtStr = "<img src=\"{0}\" alt=\"Image #{0}\" style=\"{3}\" />";
                else
                    fmtStr = "<img src=\"{0}\" alt=\"Image #{0}\" width=\"{1}\" height=\"{2}\" style=\"{3}\" />";

                QString imgHtml;
                fmt::format_to(std::back_inserter(imgHtml), fmtStr,
                               cmd.image, cmd.size.width(), cmd.size.height(), cmd.style);

                cur.insertHtml(imgHtml);
            }
            if constexpr(std::is_same_v<T, TextBufferCommand::WriteText>) {
                cur.insertText(cmd.text);
            }
        }, cmd);
    }

    m_Commands.clear();

    /* store current style and hyperlink for next time */
    m_Commands.emplace_back(TextBufferCommand::StylePush{std::move(style)});
    m_Commands.emplace_back(TextBufferCommand::HyperlinkPush{link});
}
