#ifndef BLANKWINDOW_HPP
#define BLANKWINDOW_HPP

#include "window.hpp"

namespace Glk {
    class BlankWindow : public Window {
        public:
            explicit BlankWindow(WindowController* winController, PairWindow* winParent, glui32 objRock = 0);


            void clearWindow() override {}

            [[nodiscard]] Glk::Window::Type windowType() const final {
                return Window::Blank;
            }
    };
}

#endif
