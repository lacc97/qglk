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
      mp_KeyboardInputProvider{new KeyboardInputProvider{this}},
      mp_MouseInputProvider{new MouseInputProvider{this}},
      mp_HyperlinkInputProvider{new HyperlinkInputProvider{this}} {
    assert(mp_Window);
    assert(mp_Widget);

    // we can do this because the lambdas are run inside a Glk::sendTaskToEventThread
    // ensuring the glk thread is paused and the code runs in the event thread
    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyCharInputRequested,
                     [this]() {
                         synchronize();
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequested,
                     [this]() {
                         synchronize();
                     });

    QObject::connect(mouseProvider(), &MouseInputProvider::notifyMouseInputRequested,
                     [this]() {
                         synchronize();
                     });

    QObject::connect(hyperlinkProvider(), &HyperlinkInputProvider::notifyHyperlinkInputRequested,
                     [this]() {
                         synchronize();
                     });


    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyCharInputRequestCancelled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequestCancelled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyCharInputRequestFulfilled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(keyboardProvider(), &KeyboardInputProvider::notifyLineInputRequestFulfilled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(mouseProvider(), &MouseInputProvider::notifyMouseInputRequestCancelled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(mouseProvider(), &MouseInputProvider::notifyMouseInputRequestFulfilled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(hyperlinkProvider(), &HyperlinkInputProvider::notifyHyperlinkInputRequestCancelled,
                     [this]() {
                         requestSynchronization();
                     });

    QObject::connect(hyperlinkProvider(), &HyperlinkInputProvider::notifyHyperlinkInputRequestFulfilled,
                     [this]() {
                         requestSynchronization();
                     });

    requestSynchronization();
}

Glk::WindowController::~WindowController() {
    assert(!mp_Window);
}

bool Glk::WindowController::supportsCharInput() const {
    return false;
}

bool Glk::WindowController::supportsHyperlinkInput() const {
    return false;
}

bool Glk::WindowController::supportsLineInput() const {
    return false;
}

bool Glk::WindowController::supportsMouseInput() const {
    return false;
}

void Glk::WindowController::closeWindow() {
    assert(onGlkThread());
    assert(mp_Window);

    mp_Window.reset();
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


