#include "blankwindow.hpp"

#include "window_stream_driver.hpp"

Glk::BlankWindow::BlankWindow(WindowController* winController, Glk::PairWindow* winParent, glui32 objRock)
    : Window(Type::Blank, winController, std::make_unique<qglk::stream_drivers::window>(this), winParent, objRock) {}
