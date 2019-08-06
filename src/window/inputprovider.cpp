#include "inputprovider.hpp"

#include <cstring>

#include <algorithm>

#include <QMutexLocker>
#include <utility>

#include "glk.hpp"
#include "qglk.hpp"
#include "event/eventqueue.hpp"

#include "window.hpp"


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

Glk::InputRequest::InputRequest() : QObject{nullptr}, m_Fulfilled{false}, m_Cancelled{false} {
}

void Glk::InputRequest::cancel() {
    QMutexLocker locker{&m_Mutex};

    assert(onGlkThread());

    m_Cancelled = true;

    emit cancelled();
}

Glk::CharInputRequest::CharInputRequest(bool unicode)
    : InputRequest{},
      m_Unicode{unicode},
      m_Char{0} {}

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
            doFulfill();
        } else {
            QVector<uint> ucs4 = ch.normalized(QString::NormalizationForm_KC).toUcs4();

            if(ucs4.size() == 1) {
                m_Char = ucs4[0];

                if(!m_Unicode)
                    m_Char = (m_Char < 256 ? m_Char : '?');

                doFulfill();
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
    assert(onGlkThread());

    Glk::Dispatch::registerArray(buf, maxBufLen, unicode);
}

Glk::LineInputRequest::~LineInputRequest() {
    Glk::Dispatch::unregisterArray(m_Buffer, m_MaxBufferLength, m_Unicode);
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
    {
        QMutexLocker locker{mutex()};

        assert(!isFulfilled());

        m_Terminator = terminator;
        m_Text = text;

        if(m_Unicode)
            std::for_each(m_Text.begin(), m_Text.end(), [](auto& ch) {
                ch = (ch < 256 ? ch : '?');
            });

        m_Text.truncate(m_MaxBufferLength);

        doFulfill();
    }

    emit fulfilled();
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

        doFulfill();
    }

    emit fulfilled();
}

//
//Glk::KeyboardInputProvider::KeyboardInputProvider(Glk::Window* parent_, bool characterInputProvider_,
//                                                  bool lineInputProvider_)
//    : QObject(parent_),
//      m_CharacterInputProvider(characterInputProvider_),
//      m_LineInputProvider(lineInputProvider_),
//      m_CharacterInputRequested(false),
//      m_LineInputRequested(false),
//      m_Unicode(false),
//      mp_LineInputBuffer(NULL),
//      m_LineInputBufferLength(0),
//      m_LineInputBufferPosition(0),
//      m_EchoesLine(true) {
//}
//
//void Glk::KeyboardInputProvider::requestCharInput(bool unicode) {
//    if(!m_CharacterInputProvider || m_CharacterInputRequested || m_LineInputRequested)
//        return;
//
//    m_Unicode = unicode;
//
//    m_CharacterInputRequested = true;
//
//    emit characterInputRequested();
//}
//
//void Glk::KeyboardInputProvider::cancelCharInputRequest() {
//    if(!m_CharacterInputProvider || !m_CharacterInputRequested)
//        return;
//
//    m_CharacterInputRequested = false;
//
//    emit characterInputRequestEnded(true);
//}
//
//void Glk::KeyboardInputProvider::requestLineInput(void* buf, glui32 maxlen, glui32 initlen, bool unicode) {
//    if(!m_LineInputProvider || m_CharacterInputRequested || m_LineInputRequested)
//        return;
//
//    Q_ASSERT(maxlen > 0);
//
//    mp_LineInputBuffer = buf;
//    m_LineInputBufferLength = maxlen;
//    m_LineInputBufferPosition = initlen;
//    m_Unicode = unicode;
//
//    Glk::Dispatch::registerArray(buf, maxlen, unicode);
//
//    m_LineInputRequested = true;
//
//    emit lineInputRequested();
//
//    for(glui32 ii = 0; ii < initlen; ii++) {
//        if(m_Unicode)
//            emit lineInputCharacterEntered(static_cast<glui32*>(buf)[ii]);
//        else
//            emit lineInputCharacterEntered(static_cast<unsigned char*>(buf)[ii]);
//    }
//}
//
//void Glk::KeyboardInputProvider::cancelLineInputRequest(event_t* ev) {
//    if(!m_LineInputProvider || !m_LineInputRequested) {
//        if(ev)
//            ev->type = evtype_None;
//
//        return;
//    }
//
//    m_LineInputRequested = false;
//
//    if(!ev)
//        return;
//
//    ev->type = evtype_LineInput;
//    ev->win = TO_WINID(windowPointer());
//    ev->val1 = m_LineInputBufferPosition;
//    ev->val2 = 0; // TODO line terminators
//
//    emit lineInputRequestEnded(true, mp_LineInputBuffer, m_LineInputBufferPosition, m_Unicode);
//
//    Glk::Dispatch::unregisterArray(mp_LineInputBuffer, m_LineInputBufferLength, m_Unicode);
//}
//
//bool Glk::KeyboardInputProvider::echoesLine() const {
//    return m_EchoesLine;
//}
//
//void Glk::KeyboardInputProvider::setLineEcho(bool le) {
//    m_EchoesLine = le;
//}
//
//void Glk::KeyboardInputProvider::setTerminators(glui32* keycodes, glui32 count) { // TODO do not affect pending request
//    m_Terminators.clear();
//
//    for(glui32 ii = 0; ii < count; ii++)
//        m_Terminators.insert(keycodes[ii]);
//}
//
//void Glk::KeyboardInputProvider::clearLineInputBuffer() {
//    if(!m_LineInputProvider || !m_LineInputRequested)
//        return;
//
//    while(m_LineInputBufferPosition > 0) {
//        m_LineInputBufferPosition--;
//        emit lineInputSpecialCharacterEntered(keycode_Delete, false);
//    }
//}
//
//void Glk::KeyboardInputProvider::fillLineInputBuffer(const QString& text) {
//    if(!m_LineInputProvider || !m_LineInputRequested)
//        return;
//
//    for(glui32 ch : text.toUcs4())
//        processLineInputCharacterEntered(ch, false);
//}
//
//inline glui32 switchKeyCode(int qkeycode) {
//    switch(qkeycode) {
//        case Qt::Key_Delete:
//        case Qt::Key_Backspace:
//            return keycode_Delete;
//
//        case Qt::Key_Down:
//            return keycode_Down;
//
//        case Qt::Key_End:
//            return keycode_End;
//
//        case Qt::Key_Escape:
//            return keycode_Escape;
//
//        case Qt::Key_F1:
//            return keycode_Func1;
//
//        case Qt::Key_F2:
//            return keycode_Func2;
//
//        case Qt::Key_F3:
//            return keycode_Func3;
//
//        case Qt::Key_F4:
//            return keycode_Func4;
//
//        case Qt::Key_F5:
//            return keycode_Func5;
//
//        case Qt::Key_F6:
//            return keycode_Func6;
//
//        case Qt::Key_F7:
//            return keycode_Func7;
//
//        case Qt::Key_F8:
//            return keycode_Func8;
//
//        case Qt::Key_F9:
//            return keycode_Func9;
//
//        case Qt::Key_F10:
//            return keycode_Func10;
//
//        case Qt::Key_F11:
//            return keycode_Func11;
//
//        case Qt::Key_F12:
//            return keycode_Func12;
//
//        case Qt::Key_Home:
//            return keycode_Home;
//
//        case Qt::Key_Left:
//            return keycode_Left;
//
//        case Qt::Key_PageDown:
//            return keycode_PageDown;
//
//        case Qt::Key_PageUp:
//            return keycode_PageUp;
//
//        case Qt::Key_Enter:
//        case Qt::Key_Return:
//            return keycode_Return;
//
//        case Qt::Key_Right:
//            return keycode_Right;
//
//        case Qt::Key_Tab:
//            return keycode_Tab;
//
//        case Qt::Key_Up:
//            return keycode_Up;
//
//        default:
//            return keycode_MAXVAL;
//    }
//}
//
//// TODO Move this code to event thread.
//bool Glk::KeyboardInputProvider::handleKeyEvent(int key, const QString& text) {
//    if(m_CharacterInputRequested) {
//        glui32 keyc = switchKeyCode(key);
//
//        if(keyc != keycode_MAXVAL) {
//            Glk::sendTaskToEventThread([&]() {
//                emit characterInputRequestEnded(false);
//            });
//            QGlk::getMainWindow().eventQueue().push(event_t{evtype_CharInput, TO_WINID(windowPointer()), keyc, 0});
//            m_CharacterInputRequested = false;
//        } else {
//            QVector<uint> ucs4 = text.normalized(QString::NormalizationForm_KC).toUcs4();
//
//            if(ucs4.size() == 1) {
//                glui32 ch = ucs4[0];
//
//                if(!m_Unicode)
//                    ch = (ch < 256 ? ch : '?');
//
//                Glk::sendTaskToEventThread([&]() {
//                    emit characterInputRequestEnded(false);
//                });
//                QGlk::getMainWindow().eventQueue().push(event_t{evtype_CharInput, TO_WINID(windowPointer()), ch, 0});
//                m_CharacterInputRequested = false;
//            }
//        }
//
//        return true;
//    } else if(m_LineInputRequested) {
//        glui32 keyc = switchKeyCode(key);
//
//        switch(keyc) {
//            case keycode_Delete:
//                if(m_LineInputBufferPosition > 0) {
//                    m_LineInputBufferPosition--;
//                    Glk::sendTaskToEventThread([&]() {
//                        emit lineInputSpecialCharacterEntered(keyc);
//                    });
//                }
//
//                break;
//
//            case keycode_Down:
//            case keycode_Up:
//                Glk::sendTaskToEventThread([&]() {
//                    emit lineInputSpecialCharacterEntered(keyc);
//                });
//                break;
//
//
//            case keycode_MAXVAL: {
//                QVector<uint> ucs4 = text.normalized(QString::NormalizationForm_KC).toUcs4();
//
//                if(ucs4.size() == 1)
//                    processLineInputCharacterEntered(ucs4[0]);
//
//                break;
//            }
//
//            default: {
//                if(keyc == keycode_Return || m_Terminators.contains(keyc)) {
//                    Glk::sendTaskToEventThread([&]() {
//                        emit lineInputRequestEnded(false, mp_LineInputBuffer, m_LineInputBufferPosition, m_Unicode);
//                    });
//                    QGlk::getMainWindow().eventQueue().push(
//                        event_t{evtype_LineInput, TO_WINID(windowPointer()), m_LineInputBufferPosition,
//                                keyc == keycode_Return ? 0 : keyc});
//                    Glk::Dispatch::unregisterArray(mp_LineInputBuffer, m_LineInputBufferLength, m_Unicode);
//                    m_LineInputRequested = false;
//                }
//
//                break;
//            }
//        }
//
//        return true;
//    }
//
//    return false;
//}
//
//Glk::Window* Glk::KeyboardInputProvider::windowPointer() {
//    return static_cast<Glk::Window*>(parent());
//}
//
//// TODO Move this code to event thread.
//void Glk::KeyboardInputProvider::processLineInputCharacterEntered(glui32 ucs4, bool doUpdate) {
//    if(m_LineInputBufferPosition >= m_LineInputBufferLength)
//        return;
//
//    if(!m_Unicode) {
//        unsigned char ch = (ucs4 < 256 ? ucs4 : '?');
//        static_cast<unsigned char*>(mp_LineInputBuffer)[m_LineInputBufferPosition++] = ch;
//        Glk::sendTaskToEventThread([&]() {
//            emit lineInputCharacterEntered(ch, doUpdate);
//        });
//    } else {
//        glui32 ch = ucs4;
//        static_cast<glui32*>(mp_LineInputBuffer)[m_LineInputBufferPosition++] = ch;
//        Glk::sendTaskToEventThread([&]() {
//            emit lineInputCharacterEntered(ch, doUpdate);
//        });
//    }
//
////     if(m_LineInputBufferPosition == m_LineInputBufferLength) {
////         Glk::sendTaskToEventThread([&]() {
////             emit lineInputRequestEnded(false, mp_LineInputBuffer, m_LineInputBufferPosition, m_Unicode);
////         });
////         QGlk::getMainWindow().eventQueue().push(event_t {evtype_LineInput, TO_WINID(windowPointer()), m_LineInputBufferPosition, 0});
////         Glk::Dispatch::unregisterArray(mp_LineInputBuffer, m_LineInputBufferLength, m_Unicode);
////         m_LineInputRequested = false;
////     }
//}
//
//Glk::MouseInputProvider::MouseInputProvider(Glk::Window* parent_, bool m_MouseInputProvider)
//    : QObject(parent_),
//      m_MouseInputProvider(m_MouseInputProvider),
//      m_MouseInputRequested(false) {
//}
//
//void Glk::MouseInputProvider::requestMouseInput() {
//    if(!m_MouseInputProvider || m_MouseInputRequested)
//        return;
//
//    m_MouseInputRequested = true;
//
//    emit mouseInputRequested();
//}
//
//void Glk::MouseInputProvider::cancelMouseInputRequest() {
//    if(!m_MouseInputProvider || !m_MouseInputRequested)
//        return;
//
//    m_MouseInputRequested = false;
//
//    emit mouseInputRequestEnded(true);
//}
//
//// TODO Move this code to event thread.
//bool Glk::MouseInputProvider::handleMouseEvent(QPoint pos) {
//    if(m_MouseInputRequested) {
//        QSize ws = windowPointer()->windowSize();
//
//        glui32 x = std::clamp(pos.x(), 0, ws.width() - 1);
//        glui32 y = std::clamp(pos.y(), 0, ws.height() - 1);
//
//        Glk::sendTaskToEventThread([&]() {
//            emit mouseInputRequestEnded(false);
//        });
//        QGlk::getMainWindow().eventQueue().push(event_t{evtype_MouseInput, TO_WINID(windowPointer()), x, y});
//        m_MouseInputRequested = false;
//    }
//
//    return false;
//}
//
//Glk::Window* Glk::MouseInputProvider::windowPointer() {
//    return static_cast<Glk::Window*>(parent());
//}
//
//Glk::HyperlinkInputProvider::HyperlinkInputProvider(Glk::Window* parent_, bool hyperlinkInputProvider_)
//    : QObject(parent_),
//      m_HyperlinkInputProvider(hyperlinkInputProvider_),
//      m_HyperlinkInputRequested(false) {
//}
//
//void Glk::HyperlinkInputProvider::requestHyperlinkInput() {
//    if(!m_HyperlinkInputProvider || m_HyperlinkInputRequested)
//        return;
//
//    m_HyperlinkInputRequested = true;
//
//    emit hyperlinkInputRequested();
//}
//
//void Glk::HyperlinkInputProvider::cancelHyperlinkInputRequest() {
//    if(!m_HyperlinkInputProvider || !m_HyperlinkInputRequested)
//        return;
//
//    m_HyperlinkInputRequested = false;
//
//    emit hyperlinkInputRequestEnded(true);
//}
//
//// TODO Move this code to event thread.
//void Glk::HyperlinkInputProvider::handleHyperlinkClicked(glui32 linkval) {
//    if(m_HyperlinkInputRequested) {
//        Glk::sendTaskToEventThread([&]() {
//            emit hyperlinkInputRequestEnded(false);
//        });
//        QGlk::getMainWindow().eventQueue().push(event_t{evtype_Hyperlink, TO_WINID(windowPointer()), linkval, 0});
//        m_HyperlinkInputRequested = false;
//    }
//}
//
//Glk::Window* Glk::HyperlinkInputProvider::windowPointer() {
//    return static_cast<Glk::Window*>(parent());
//}
