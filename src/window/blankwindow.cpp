#include "blankwindow.hpp"

#include "stream/nulldevice.hpp"

Glk::BlankWindow::BlankWindow(WindowController* winController, Glk::PairWindow* winParent, glui32 objRock)
    : Window(Type::Blank, winController, std::make_unique<WindowBuf>(this), winParent, objRock) {}
