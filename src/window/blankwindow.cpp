#include "blankwindow.hpp"

#include "stream/nulldevice.hpp"

Glk::BlankWindow::BlankWindow(glui32 rock_) : Window(new NullDevice(), rock_) {}
