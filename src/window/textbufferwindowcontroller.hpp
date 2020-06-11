#ifndef TEXTBUFFERWINDOWCONTROLLER_HPP
#define TEXTBUFFERWINDOWCONTROLLER_HPP

#include <string_view>
#include <variant>
#include <vector>

#include "style.hpp"
#include "windowcontroller.hpp"

class QTextBlockFormat;

class QTextCharFormat;

class QTextCursor;

class QTextDocument;

namespace Glk {
    namespace TextBufferCommand {
        struct Clear {};

        struct FlowBreak {};

        struct HyperlinkPush {
            glui32 link;
        };

        struct StylePush {
            Style style;
        };

        struct WriteImage {
            glui32 image;
            QSize size;
            std::u16string_view style;
        };

        struct WriteText {
            QString text;
        };
    }

    class TextBufferWindowController : public WindowController {
            using Command = std::variant<
                    TextBufferCommand::Clear,
                    TextBufferCommand::FlowBreak,
                    TextBufferCommand::HyperlinkPush,
                    TextBufferCommand::StylePush,
                    TextBufferCommand::WriteImage,
                    TextBufferCommand::WriteText
            >;

        public:
            TextBufferWindowController(PairWindow* winParent, glui32 winRock);

            ~TextBufferWindowController() override;


            void synchronize() override;

            QPoint glkPos(const QPoint& qtPos) const override;

            [[nodiscard]] QSize glkSize() const override;

            [[nodiscard]] QSize toQtSize(const QSize& glk) const override;

            [[nodiscard]] bool supportsCharInput() const override;

            [[nodiscard]] bool supportsHyperlinkInput() const override;

            [[nodiscard]] bool supportsLineInput() const override;


            void pushCommand(Command cmd);

        private:
            [[nodiscard]] static QWidget* createWidget();


            void synchronizeInputStyle();

            void synchronizeText();


            std::vector<Command> m_Commands;
    };
}


#endif //TEXTBUFFERWINDOWCONTROLLER_HPP
