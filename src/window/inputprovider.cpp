#include "inputprovider.hpp"

#include <cstring>

#include <algorithm>

#include <QMutexLocker>
#include <memory>
#include <utility>

#include "glk.hpp"
#include "qglk.hpp"
#include "event/eventqueue.hpp"
#include "log/log.hpp"

#include "window.hpp"
#include "windowwidget.hpp"


namespace {
    glui32 toGlkKey(Qt::Key key) {
        switch(key) {
            case Qt::Key_Delete:
            case Qt::Key_Backspace:
                return keycode_Delete;

            case Qt::Key_Down:
                return keycode_Down;

            case Qt::Key_End:
                return keycode_End;

            case Qt::Key_Escape:
                return keycode_Escape;

            case Qt::Key_F1:
                return keycode_Func1;

            case Qt::Key_F2:
                return keycode_Func2;

            case Qt::Key_F3:
                return keycode_Func3;

            case Qt::Key_F4:
                return keycode_Func4;

            case Qt::Key_F5:
                return keycode_Func5;

            case Qt::Key_F6:
                return keycode_Func6;

            case Qt::Key_F7:
                return keycode_Func7;

            case Qt::Key_F8:
                return keycode_Func8;

            case Qt::Key_F9:
                return keycode_Func9;

            case Qt::Key_F10:
                return keycode_Func10;

            case Qt::Key_F11:
                return keycode_Func11;

            case Qt::Key_F12:
                return keycode_Func12;

            case Qt::Key_Home:
                return keycode_Home;

            case Qt::Key_Left:
                return keycode_Left;

            case Qt::Key_PageDown:
                return keycode_PageDown;

            case Qt::Key_PageUp:
                return keycode_PageUp;

            case Qt::Key_Enter:
            case Qt::Key_Return:
                return keycode_Return;

            case Qt::Key_Right:
                return keycode_Right;

            case Qt::Key_Tab:
                return keycode_Tab;

            case Qt::Key_Up:
                return keycode_Up;

            default:
                return keycode_MAXVAL;
        }
    }

    std::vector<Qt::Key> toQtKey(glui32 glkcode) {
        switch(glkcode) {
            case keycode_Delete:
                return {Qt::Key_Backspace, Qt::Key_Delete};

            case keycode_Down:
                return {Qt::Key_Down};

            case keycode_End:
                return {Qt::Key_End};

            case keycode_Escape:
                return {Qt::Key_Escape};

            case keycode_Func1:
                return {Qt::Key_F1};

            case keycode_Func2:
                return {Qt::Key_F2};

            case keycode_Func3:
                return {Qt::Key_F3};

            case keycode_Func4:
                return {Qt::Key_F4};

            case keycode_Func5:
                return {Qt::Key_F5};

            case keycode_Func6:
                return {Qt::Key_F6};

            case keycode_Func7:
                return {Qt::Key_F7};

            case keycode_Func8:
                return {Qt::Key_F8};

            case keycode_Func9:
                return {Qt::Key_F9};

            case keycode_Func10:
                return {Qt::Key_F10};

            case keycode_Func11:
                return {Qt::Key_F11};

            case keycode_Func12:
                return {Qt::Key_F12};

            case keycode_Home:
                return {Qt::Key_Home};

            case keycode_Left:
                return {Qt::Key_Left};

            case keycode_PageDown:
                return {Qt::Key_PageDown};

            case keycode_PageUp:
                return {Qt::Key_PageUp};

            case keycode_Return:
                return {Qt::Key_Enter, Qt::Key_Return};

            case keycode_Right:
                return {Qt::Key_Right};

            case keycode_Tab:
                return {Qt::Key_Tab};

            case keycode_Up:
                return {Qt::Key_Up};

            default:
                return {};
        }
    }
}

Glk::InputRequest::InputRequest()
    : QObject{nullptr},
      m_Fulfilled{false},
      m_Cancelled{false} {
}

void Glk::InputRequest::cancel() {
    QMutexLocker locker{&m_Mutex};

    m_Cancelled = true;

    emit cancelled();
}

bool Glk::InputRequest::isPending() const {
//    QMutexLocker locker{m_Mutex};
    return !m_Fulfilled && !m_Cancelled;
}

Glk::CharInputRequest::CharInputRequest(bool unicode)
    : InputRequest{},
      m_Unicode{unicode},
      m_Char{0} {
    assert(onEventThread());
}

event_t Glk::CharInputRequest::generateEvent(Glk::Window* win) {
    QMutexLocker locker{mutex()};

    assert(onEventThread());
    assert(!isCancelled() && isFulfilled());

    return event_t{evtype_CharInput, TO_WINID(win), m_Char, 0};
}

void Glk::CharInputRequest::fulfill(Qt::Key key, const QString& ch) {
    {
        QMutexLocker locker{mutex()};

        assert(!isFulfilled());

        if(isCancelled())
            return;

        glui32 glkKey = toGlkKey(key);

        if(glkKey != keycode_MAXVAL) {
            m_Char = glkKey;
            InputRequest::fulfill();
        } else {
            QVector<uint> ucs4 = ch.normalized(QString::NormalizationForm_KC).toUcs4();

            if(ucs4.size() == 1) {
                m_Char = ucs4[0];

                if(!m_Unicode)
                    m_Char = (m_Char < 256 ? m_Char : '?');

                InputRequest::fulfill();
            }
        }
    }

    emit fulfilled();
}

std::vector<Qt::Key> Glk::LineInputRequest::toQtKeyTerminators(glui32 keycode) {
    return toQtKey(keycode);
}

Glk::LineInputRequest::LineInputRequest(void* buf, glui32 maxBufLen, glui32 initBufLen, bool unicode,
                                        std::set<Qt::Key> terms, bool echoes)
    : InputRequest{},
      m_Unicode{unicode},
      m_Buffer{buf},
      m_MaxBufferLength{maxBufLen},
      m_Terminators{std::move(terms)},
      m_Echoes{echoes},
      m_Text{m_Unicode ? QString::fromUcs4(buffer<glui32>(), initBufLen) : QString::fromLatin1(buffer<char>(),
                                                                                               initBufLen)},
      m_Terminator{Qt::Key_unknown} {
    assert(onEventThread());

    QGlk::getMainWindow().dispatch().registerArray(buf, maxBufLen, unicode);
}

Glk::LineInputRequest::~LineInputRequest() {
    QGlk::getMainWindow().dispatch().unregisterArray(m_Buffer, m_MaxBufferLength, m_Unicode);
}

event_t Glk::LineInputRequest::generateEvent(Glk::Window* win) {
    QMutexLocker locker{mutex()};

    assert(onEventThread());
    assert(isFulfilled() || isCancelled());

    glui32 writtenLength;

    if(m_Unicode) {
        const auto ucs4 = m_Text.toUcs4();
        static_assert(std::is_same_v<std::make_unsigned_t<decltype(ucs4)::value_type>, std::make_unsigned_t<glui32>>);
        std::memcpy(buffer<glui32>(), ucs4.data(), sizeof(glui32) * ucs4.size());
        writtenLength = ucs4.size();
    } else {
        const auto latin1 = m_Text.toLatin1();
        static_assert(std::is_same_v<std::make_unsigned_t<decltype(latin1)::value_type>, std::make_unsigned_t<char>>);
        std::memcpy(buffer<char>(), latin1.data(), sizeof(char) * latin1.size());
        writtenLength = latin1.size();
    }

    glui32 glkTerminator = toGlkKey(m_Terminator);

    return event_t{evtype_LineInput, TO_WINID(win), writtenLength,
                   (glkTerminator == keycode_Return || glkTerminator == keycode_MAXVAL) ? 0 : glkTerminator};
}

void Glk::LineInputRequest::fulfill(Qt::Key terminator, const QString& text) {
    assert(onEventThread());

    if(isPending()) {
        {
            QMutexLocker locker{mutex()};

            m_Terminator = terminator;
            m_Text = text;

            if(m_Unicode)
                std::for_each(m_Text.begin(), m_Text.end(), [](auto& ch) {
                    ch = (ch < 256 ? ch : '?');
                });

            m_Text.truncate(m_MaxBufferLength);

            if(!isCancelled())
                InputRequest::fulfill();
        }

        if(!isCancelled())
            emit fulfilled();
    }
}

event_t Glk::MouseInputRequest::generateEvent(Glk::Window* win) {
    QMutexLocker locker{mutex()};

    assert(onEventThread());
    assert(isFulfilled());

    QPoint glkPos = win->controller()->glkPos(m_ClickPos);

    return event_t{evtype_MouseInput, TO_WINID(win), static_cast<glui32>(glkPos.x()), static_cast<glui32>(glkPos.y())};
}

void Glk::MouseInputRequest::fulfill(const QPoint& qtpos) {
    {
        QMutexLocker locker{mutex()};

        assert(!isFulfilled());

        m_ClickPos = qtpos;

        if(!isCancelled())
            InputRequest::fulfill();
    }

    if(!isCancelled())
        emit fulfilled();
}

event_t Glk::HyperlinkInputRequest::generateEvent(Glk::Window* win) {
    QMutexLocker locker{mutex()};

    assert(onEventThread());
    assert(isFulfilled());

    return event_t{evtype_Hyperlink, TO_WINID(win), linkValue()};
}

void Glk::HyperlinkInputRequest::fulfill(glui32 linkval) {
    {
        QMutexLocker locker{mutex()};

        m_LinkValue = linkval;

        if(!isCancelled())
            InputRequest::fulfill();
    }

    if(!isCancelled())
        emit fulfilled();
}

Glk::InputProvider::InputProvider(Glk::WindowController* winController)
    : QObject{},
      mp_WindowsController{winController} {
    assert(mp_WindowsController);
}

Glk::KeyboardInputProvider::KeyboardInputProvider(Glk::WindowController* winController)
    : InputProvider{winController},
      mp_CharInputRequest{nullptr},
      m_LineInputEcho{true},
      m_LineInputTerminators{},
      mp_LineInputRequest{nullptr} {}

void Glk::KeyboardInputProvider::requestCharInput(bool unicode) {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        assert(dynamic_cast<WindowWidget*>(controller()->widget()));
#if !defined(NDEBUG)
        assert(!lineInputRequest() || !lineInputRequest()->isPending());
        assert(!charInputRequest() || !charInputRequest()->isPending());
#else
        if(lineInputRequest() && lineInputRequest()->isPending()) {
            spdlog::error("Char input requested on window {} but there is already a pending char input request.",
                          wrap::ptr(controller()->window()));
            return;
        }
        if(charInputRequest() && charInputRequest()->isPending()) {
            spdlog::error("Char input requested on window {} but there is already a pending line input request.",
                          wrap::ptr(controller()->window()));
            return;
        }
#endif

        emit notifyCharInputRequested();

        mp_CharInputRequest = std::make_unique<CharInputRequest>(unicode);

        controller()->widget<WindowWidget>()->requestCharInput();

        QObject::connect(controller()->widget<WindowWidget>(), &WindowWidget::characterInput,
                         charInputRequest(), &CharInputRequest::fulfill,
                         Qt::DirectConnection);

        QObject::connect(charInputRequest(), &CharInputRequest::cancelled,
                         controller()->widget<WindowWidget>(), &WindowWidget::cancelCharInput,
                         Qt::DirectConnection);

        QObject::connect(charInputRequest(), &CharInputRequest::fulfilled,
                         this, &KeyboardInputProvider::onCharInputFulfilled,
                         Qt::DirectConnection);
    });
}

void Glk::KeyboardInputProvider::cancelCharInputRequest() {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        if(charInputRequest() && !charInputRequest()->isCancelled()) {
            charInputRequest()->cancel();

            emit notifyCharInputRequestCancelled();

            mp_CharInputRequest.reset();
        }
    });
}

void Glk::KeyboardInputProvider::requestLineInput(void* buf, glui32 maxLen, glui32 initLen, bool unicode) {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        assert(dynamic_cast<WindowWidget*>(controller()->widget()));
#if !defined(NDEBUG)
        assert(!charInputRequest() || !charInputRequest()->isPending());
        assert(!lineInputRequest() || !lineInputRequest()->isPending());
#else
        if(charInputRequest() && charInputRequest()->isPending()) {
            spdlog::error("Line input requested on window {} but there is already a pending char input request.",
                          wrap::ptr(controller()->window()));
            return;
        }
        if(lineInputRequest() && lineInputRequest()->isPending()) {
            spdlog::error("Line input requested on window {} but there is already a pending line input request.",
                          wrap::ptr(controller()->window()));
            return;
        }
#endif

        emit notifyLineInputRequested();

        mp_LineInputRequest = std::make_unique<LineInputRequest>(buf, maxLen, initLen, unicode,
                                                                 m_LineInputTerminators, m_LineInputEcho);

        controller()->widget<WindowWidget>()->requestLineInput(maxLen, m_LineInputTerminators);

        QObject::connect(controller()->widget<WindowWidget>(), &WindowWidget::lineInput,
                         lineInputRequest(), &LineInputRequest::fulfill,
                         Qt::DirectConnection);

        QObject::connect(lineInputRequest(), &LineInputRequest::cancelled,
                         controller()->widget<WindowWidget>(), &WindowWidget::cancelLineInput,
                         Qt::DirectConnection);

        QObject::connect(lineInputRequest(), &LineInputRequest::fulfilled,
                         this, &KeyboardInputProvider::onLineInputFulfilled,
                         Qt::DirectConnection);
    });
}

void Glk::KeyboardInputProvider::cancelLineInputRequest(event_t* ev) {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        if(lineInputRequest() && !lineInputRequest()->isCancelled()) {
            lineInputRequest()->cancel();

            event_t lineInputEvent;
            if(lineInputRequest()->isFulfilled()) {
                // event has been logged but not yet retrieved by glk_select
                lineInputEvent = QGlk::getMainWindow().eventQueue().popLineEvent(controller()->window());
                assert(lineInputEvent.type == evtype_LineInput);
            } else {
                lineInputEvent = lineInputRequest()->generateEvent(controller()->window());
            }

            if(ev)
                *ev = lineInputEvent;

            emit notifyLineInputRequestCancelled(lineInputRequest()->text(), lineInputRequest()->lineEchoes());


            mp_LineInputRequest.reset();
        }
    });
}

void Glk::KeyboardInputProvider::onCharInputFulfilled() {
    assert(onEventThread());

    emit notifyCharInputRequestFulfilled();

    QGlk::getMainWindow().eventQueue().push(charInputRequest()->generateEvent(controller()->window()));

    mp_CharInputRequest.reset();
}

void Glk::KeyboardInputProvider::onLineInputFulfilled() {
    assert(onEventThread());

    emit notifyLineInputRequestFulfilled(lineInputRequest()->text(), lineInputRequest()->lineEchoes());

    QGlk::getMainWindow().eventQueue().push(lineInputRequest()->generateEvent(controller()->window()));

    mp_LineInputRequest.reset();
}

Glk::MouseInputProvider::MouseInputProvider(Glk::WindowController* winController)
    : InputProvider{winController},
      mp_MouseInputRequest{nullptr} {

}

void Glk::MouseInputProvider::requestMouseInput() {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        assert(dynamic_cast<WindowWidget*>(controller()->widget()));
#if !defined(NDEBUG)
        assert(!mouseInputRequest() || !mouseInputRequest()->isPending());
#else
        if(mouseInputRequest() && mouseInputRequest()->isPending()) {
            spdlog::error("Mouse input requested on window {} but there is already a pending mouse input request.",
                          wrap::ptr(controller()->window()));
            return;
        }
#endif

        emit notifyMouseInputRequested();

        mp_MouseInputRequest = std::make_unique<MouseInputRequest>();

        controller()->widget<WindowWidget>()->requestMouseInput();

        QObject::connect(controller()->widget<WindowWidget>(), &WindowWidget::mouseInput,
                         mouseInputRequest(), &MouseInputRequest::fulfill,
                         Qt::DirectConnection);

        QObject::connect(mouseInputRequest(), &MouseInputRequest::cancelled,
                         controller()->widget<WindowWidget>(), &WindowWidget::cancelMouseInput,
                         Qt::DirectConnection);

        QObject::connect(mouseInputRequest(), &MouseInputRequest::fulfilled,
                         this, &MouseInputProvider::onMouseInputFulfilled,
                         Qt::DirectConnection);
    });
}

void Glk::MouseInputProvider::cancelMouseInputRequest() {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        if(mouseInputRequest() && !mouseInputRequest()->isCancelled()) {
            mouseInputRequest()->cancel();

            emit notifyMouseInputRequestCancelled();

            mp_MouseInputRequest.reset();
        }
    });
}

void Glk::MouseInputProvider::onMouseInputFulfilled() {
    assert(onEventThread());

    emit notifyMouseInputRequestFulfilled(mouseInputRequest()->point());

    QGlk::getMainWindow().eventQueue().push(mouseInputRequest()->generateEvent(controller()->window()));

    mp_MouseInputRequest.reset();
}

Glk::HyperlinkInputProvider::HyperlinkInputProvider(Glk::WindowController* winController)
    : InputProvider{winController},
      mp_HyperlinkInputRequest{nullptr} {
}

void Glk::HyperlinkInputProvider::requestHyperlinkInput() {
    assert(onGlkThread());


    Glk::sendTaskToEventThread([this]() {
        assert(dynamic_cast<WindowWidget*>(controller()->widget()));
#if !defined(NDEBUG)
        assert(!hyperlinkInputRequest() || !hyperlinkInputRequest()->isPending());
#else
        if(hyperlinkInputRequest() && hyperlinkInputRequest()->isPending()) {
            spdlog::error("Hyperlink input requested on window {} but there is already a pending hyperlink input request.",
                          wrap::ptr(controller()->window()));
            return;
        }
#endif

        emit notifyHyperlinkInputRequested();

        mp_HyperlinkInputRequest = std::make_unique<HyperlinkInputRequest>();

        controller()->widget<WindowWidget>()->requestHyperlinkInput();

        QObject::connect(controller()->widget<WindowWidget>(), &WindowWidget::hyperlinkInput,
                         hyperlinkInputRequest(), &HyperlinkInputRequest::fulfill,
                         Qt::DirectConnection);

        QObject::connect(hyperlinkInputRequest(), &HyperlinkInputRequest::cancelled,
                         controller()->widget<WindowWidget>(), &WindowWidget::cancelHyperlinkInput,
                         Qt::DirectConnection);

        QObject::connect(hyperlinkInputRequest(), &HyperlinkInputRequest::fulfilled,
                         this, &HyperlinkInputProvider::onHyperlinkInputFulfilled,
                         Qt::DirectConnection);
    });
}

void Glk::HyperlinkInputProvider::cancelHyperlinkInputRequest() {
    assert(onGlkThread());

    Glk::sendTaskToEventThread([=]() {
        if(hyperlinkInputRequest() && !hyperlinkInputRequest()->isCancelled()) {
            hyperlinkInputRequest()->cancel();

            emit notifyHyperlinkInputRequestCancelled();

            mp_HyperlinkInputRequest.reset();
        }
    });
}

void Glk::HyperlinkInputProvider::onHyperlinkInputFulfilled() {
    assert(onEventThread());

    emit notifyHyperlinkInputRequestFulfilled(hyperlinkInputRequest()->linkValue());

    QGlk::getMainWindow().eventQueue().push(hyperlinkInputRequest()->generateEvent(controller()->window()));

    mp_HyperlinkInputRequest.reset();
}
