#ifndef BLANKWINDOWCONTROLLER_HPP
#define BLANKWINDOWCONTROLLER_HPP

#include "windowcontroller.hpp"
#include "pairwindow.hpp"

namespace Glk {
    class BlankWindowController : public WindowController {
        public:
            BlankWindowController(PairWindow* parent, glui32 rock);

            ~BlankWindowController() override = default;

            QPoint glkPos(const QPoint& qtPos) const override;

            [[nodiscard]] QSize glkSize() const override;

            QSize toQtSize(const QSize& glk) const override;

        private:
            static QWidget* createWidget();
    };
}

#endif