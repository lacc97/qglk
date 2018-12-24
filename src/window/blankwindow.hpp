#ifndef BLANKWINDOW_HPP
#define BLANKWINDOW_HPP

#include "window.hpp"

namespace Glk {
    class BlankWindow : public Window {
            Q_OBJECT
        public:
            BlankWindow(glui32 rock_ = 0);

            Glk::Window::Type windowType() const override {
                return Window::Blank;
            }

            void clearWindow() override {
            }

            QSize windowSize() const override {
                return QSize();
            }

        protected:
            QSize pixelsToUnits(const QSize& pixels) const override {
                return pixels;
            }
            QSize unitsToPixels(const QSize& units) const override {
                return units;
            }
    };
}

#endif
