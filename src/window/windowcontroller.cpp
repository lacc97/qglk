#include "windowcontroller.hpp"

#include "qglk.hpp"
#include "log/log.hpp"
#include "thread/taskrequest.hpp"

#include "blankwindowcontroller.hpp"
#include "graphicswindowcontroller.hpp"
#include "textbufferwindowcontroller.hpp"
#include "textgridwindowcontroller.hpp"
#include "window.hpp"

Glk::WindowController* Glk::WindowController::createController(glui32 wintype, PairWindow* parent, glui32 rock) {
    switch(wintype) {
        case Window::Blank:
            return new BlankWindowController(parent, rock);
        case Window::Graphics:
            return new GraphicsWindowController(parent, rock);
        case Window::TextBuffer:
            return new TextBufferWindowController(parent, rock);
        case Window::TextGrid:
            return new TextGridWindowController(parent, rock);
        default:
            return nullptr;
    }
}

Glk::WindowController::WindowController(Glk::Window* win, QWidget* widg)
    : mp_Window{win},
      mp_Widget{widg},
      m_RequiresSynchronization{false},
      m_LineInputEcho{true},
      mp_CharInputRequest{nullptr},
      mp_LineInputRequest{nullptr},
      mp_MouseInputRequest{nullptr} {
    assert(mp_Window);
    assert(mp_Widget);

    requestSynchronization();
}

Glk::WindowController::~WindowController() {
    assert(!mp_Window);
}

void Glk::WindowController::cancelCharInput() {
    log_warn() << "Failed to cancel char input event. Window " << window() << " ("
               << Glk::Window::windowsTypeString(window()->windowType())
               << ") does not accept char input.";
}

event_t Glk::WindowController::cancelLineInput() {
    log_warn() << "Failed to cancel line input event. Window " << window() << " ("
               << Glk::Window::windowsTypeString(window()->windowType())
               << ") does not accept line input.";

    return event_t{};
}

void Glk::WindowController::cancelMouseInput() {
    log_warn() << "Failed to cancel mouse input event. Window " << window() << " ("
               << Glk::Window::windowsTypeString(window()->windowType())
               << ") does not accept mouse input.";
}

void Glk::WindowController::closeWindow() {
    assert(onGlkThread());
    assert(mp_Window);

    mp_Window.reset();
}

void Glk::WindowController::requestCharInput(bool unicode) {
    log_warn() << "Window " << window() << " (" << Glk::Window::windowsTypeString(window()->windowType())
               << ") does not accept char input.";
}

void Glk::WindowController::requestLineInput(void* buf, glui32 maxLen, glui32 initLen, bool unicode) {
    log_warn() << "Window " << window() << " (" << Glk::Window::windowsTypeString(window()->windowType())
               << ") does not accept line input.";
}

void Glk::WindowController::requestMouseInput() {
    log_warn() << "Window " << window() << " (" << Glk::Window::windowsTypeString(window()->windowType())
               << ") does not accept mouse input.";
}

void Glk::WindowController::requestSynchronization() {
    m_RequiresSynchronization = true;
    QGlk::getMainWindow().eventQueue().requestImmediateSynchronization();
}

void Glk::WindowController::synchronize() {
    assert(onEventThread());

    widget()->update();

    m_RequiresSynchronization = false;
}


