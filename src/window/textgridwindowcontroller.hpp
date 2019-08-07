#ifndef TEXTGRIDWINDOWCONTROLLER_HPP
#define TEXTGRIDWINDOWCONTROLLER_HPP


#include "windowcontroller.hpp"

namespace Glk {
    class TextGridWindowController : public WindowController {
        public:
            TextGridWindowController(PairWindow* winParent, glui32 winRock);


            bool supportsCharInput() const override;

            void synchronize() override;

            QPoint glkPos(const QPoint& qtPos) const override;

            [[nodiscard]] QSize glkSize() const override;

            QSize toQtSize(const QSize& glk) const override;

        private:
            [[nodiscard]] static QWidget* createWidget();
    };
}


#endif //TEXTGRIDWINDOWCONTROLLER_HPP
