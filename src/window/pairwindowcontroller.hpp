#ifndef PAIRWINDOWCONTROLLER_HPP
#define PAIRWINDOWCONTROLLER_HPP

#include "windowcontroller.hpp"

namespace Glk {
    class WindowArrangement;

    class PairWindowController : public WindowController {
        public:
            static Glk::PairWindowController*
            createController(Glk::Window* winKey, Glk::Window* winFirst, Glk::Window* winSecond,
                             Glk::WindowArrangement* winArrangement, Glk::PairWindow* winParent);


            void closeWindow() override;

            void synchronize() override;

            QPoint glkPos(const QPoint& qtPos) const override;

            [[nodiscard]] QSize glkSize() const override;

            QSize toQtSize(const QSize& glk) const override;

        private:
            [[nodiscard]] static QWidget* createWidget();


            PairWindowController(Glk::Window* winKey, Glk::Window* winFirst, Glk::Window* winSecond,
                                 Glk::WindowArrangement* winArrangement, Glk::PairWindow* winParent);
    };
}


#endif //PAIRWINDOWCONTROLLER_HPP
