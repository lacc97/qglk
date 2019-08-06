#ifndef TEXTBUFFERWINDOWCONTROLLER_HPP
#define TEXTBUFFERWINDOWCONTROLLER_HPP


#include "windowcontroller.hpp"

class QTextBlockFormat;

class QTextCharFormat;

class QTextCursor;

class QTextDocument;

namespace Glk {
    class TextBufferWindowController : public WindowController {
        public:
            TextBufferWindowController(PairWindow* winParent, glui32 winRock);

            ~TextBufferWindowController() override;


            void cancelCharInput() override;

            event_t cancelLineInput() override;

            void requestCharInput(bool unicode) override;

            void requestLineInput(void* buf, glui32 maxLen, glui32 initLen, bool unicode) override;

            void synchronize() override;

            QPoint glkPos(const QPoint& qtPos) const override;

            [[nodiscard]] QSize glkSize() const override;

            [[nodiscard]] QSize toQtSize(const QSize& glk) const override;


            void clearDocument();

            void flowBreak();

            void writeHTML(const QString& html);

            void writeString(const QString& str, const QTextCharFormat& chFmt, const QTextBlockFormat& blkFmt);

        private:
            [[nodiscard]] static QWidget* createWidget();


            QTextDocument* mp_EventThreadDocument;
            QTextCursor* mp_Cursor;


            void synchronizeText();
    };
}


#endif //TEXTBUFFERWINDOWCONTROLLER_HPP
