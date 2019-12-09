#include "blankwindow.hpp"

#include "stream/nulldevice.hpp"

Glk::BlankWindow::BlankWindow(WindowController* winController, Glk::PairWindow* winParent, glui32 objRock)
    : Window(Type::Blank, winController, new WindowDevice{this}, winParent, objRock) {}
