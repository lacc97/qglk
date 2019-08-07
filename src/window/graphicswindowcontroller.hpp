#ifndef GRAPHICSWINDOWCONTROLLER_HPP
#define GRAPHICSWINDOWCONTROLLER_HPP

#include <QColor>
#include <QPixmap>

#include "windowcontroller.hpp"

namespace Glk {
    class GraphicsWindowController : public WindowController {
        public:
            GraphicsWindowController(PairWindow* parent, glui32 rock);

            ~GraphicsWindowController() override = default;


            [[nodiscard]] bool supportsMouseInput() const override;

            void synchronize() override;

            [[nodiscard]] QPoint glkPos(const QPoint& qtPos) const override;

            [[nodiscard]] QSize glkSize() const override;

            [[nodiscard]] QSize toQtSize(const QSize& glk) const override;


            [[nodiscard]] QColor getDefaultBackgroundColor() const;

        private:
            [[nodiscard]] static QWidget* createWidget();
    };
}


#endif //GRAPHICSWINDOWCONTROLLER_HPP
