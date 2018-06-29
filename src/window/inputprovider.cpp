#include "inputprovider.hpp"

#include <algorithm>

#include "glk.hpp"
#include "qglk.hpp"
#include "event/eventqueue.hpp"

Glk::KeyboardInputProvider::KeyboardInputProvider(Glk::Window* parent_, bool characterInputProvider_, bool lineInputProvider_) : QObject(parent_), m_CharacterInputProvider(characterInputProvider_), m_LineInputProvider(lineInputProvider_), m_CharacterInputRequested(false), m_LineInputRequested(false), m_Unicode(false), mp_LineInputBuffer(NULL), m_LineInputBufferLength(0), m_LineInputBufferPosition(0), m_EchoesLine(true) {
}

void Glk::KeyboardInputProvider::requestCharInput(bool unicode) {
    if(!m_CharacterInputProvider || m_CharacterInputRequested || m_LineInputRequested)
        return;

    m_Unicode = unicode;

    m_CharacterInputRequested = true;

    emit characterInputRequested();
}

void Glk::KeyboardInputProvider::cancelCharInputRequest() {
    if(!m_CharacterInputProvider || !m_CharacterInputRequested)
        return;

    m_CharacterInputRequested = false;

    emit characterInputRequestEnded(true);
}

void Glk::KeyboardInputProvider::requestLineInput(void* buf, glui32 maxlen, glui32 initlen, bool unicode) {
    if(!m_LineInputProvider || m_CharacterInputRequested || m_LineInputRequested)
        return;

    Q_ASSERT(maxlen > 0);

    mp_LineInputBuffer = buf;
    m_LineInputBufferLength = maxlen;
    m_LineInputBufferPosition = initlen;
    m_Unicode = unicode;

    Glk::Dispatch::registerArray(buf, maxlen, unicode);

    m_LineInputRequested = true;

    emit lineInputRequested();

    for(glui32 ii = 0; ii < initlen; ii++) {
        if(m_Unicode)
            emit lineInputCharacterEntered(static_cast<glui32*>(buf)[ii]);
        else
            emit lineInputCharacterEntered(static_cast<unsigned char*>(buf)[ii]);
    }
}

void Glk::KeyboardInputProvider::cancelLineInputRequest(event_t* ev) {
    if(!m_LineInputProvider || !m_LineInputRequested)
        return;

    m_LineInputRequested = false;

    if(!ev)
        return;

    ev->type = evtype_LineInput;
    ev->win = TO_WINID(this);
    ev->val1 = m_LineInputBufferPosition;
    ev->val2 = 0; // TODO line terminators

    emit lineInputRequestEnded(true, mp_LineInputBuffer, m_LineInputBufferPosition, m_Unicode);

    Glk::Dispatch::unregisterArray(mp_LineInputBuffer, m_LineInputBufferLength, m_Unicode);
}

bool Glk::KeyboardInputProvider::echoesLine() const {
    return m_EchoesLine;
}

void Glk::KeyboardInputProvider::setLineEcho(bool le) {
    m_EchoesLine = le;
}

void Glk::KeyboardInputProvider::setTerminators(glui32* keycodes, glui32 count) { // TODO do not affect pending request
    m_Terminators.clear();

    for(glui32 ii = 0; ii < count; ii++)
        m_Terminators.insert(keycodes[ii]);
}

inline glui32 switchKeyCode(int qkeycode) {
    switch(qkeycode) {
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

bool Glk::KeyboardInputProvider::handleKeyEvent(QKeyEvent* ev) {
    if(m_CharacterInputRequested) {
        glui32 keyc = switchKeyCode(ev->key());

        if(keyc != keycode_MAXVAL) {
            emit characterInputRequestEnded(false);
            QGlk::getMainWindow().eventQueue().push(event_t {evtype_CharInput, TO_WINID(windowPointer()), keyc, 0});
            m_CharacterInputRequested = false;
        } else {
            QVector<uint> ucs4 = ev->text().normalized(QString::NormalizationForm_KC).toUcs4();

            if(ucs4.size() == 1) {
                glui32 ch = ucs4[0];

                if(!m_Unicode)
                    ch = (ch < 256 ? ch : '?');

                emit characterInputRequestEnded(false);
                QGlk::getMainWindow().eventQueue().push(event_t {evtype_CharInput, TO_WINID(windowPointer()), ch, 0});
                m_CharacterInputRequested = false;
            }
        }

        return true;
    } else if(m_LineInputRequested) {
        glui32 keyc = switchKeyCode(ev->key());

        switch(keyc) {
            case keycode_Delete: {
                if(m_LineInputBufferPosition > 0) {
                    m_LineInputBufferPosition--;
                    emit lineInputSpecialCharacterEntered(keyc);
                }

                break;
            }

            case keycode_MAXVAL: {
                QVector<uint> ucs4 = ev->text().normalized(QString::NormalizationForm_KC).toUcs4();

                if(ucs4.size() == 1) {
                    if(!m_Unicode) {
                        unsigned char ch = (ucs4[0] < 256 ? ucs4[0] : '?');
                        static_cast<unsigned char*>(mp_LineInputBuffer)[m_LineInputBufferPosition++] = ch;
                        emit lineInputCharacterEntered(ch);
                    } else {
                        glui32 ch = ucs4[0];
                        static_cast<glui32*>(mp_LineInputBuffer)[m_LineInputBufferPosition++] = ch;
                        emit lineInputCharacterEntered(ch);
                    }

                    if(m_LineInputBufferPosition == m_LineInputBufferLength) {
                        emit lineInputRequestEnded(false, mp_LineInputBuffer, m_LineInputBufferPosition, m_Unicode);
                        Glk::sendTaskToGlkThread([&] {
                            Glk::Dispatch::unregisterArray(mp_LineInputBuffer, m_LineInputBufferLength, m_Unicode);
                        });
                        QGlk::getMainWindow().eventQueue().push(event_t {evtype_LineInput, TO_WINID(windowPointer()), m_LineInputBufferPosition, 0});
                        m_LineInputRequested = false;
                    }
                }

                break;
            }

            default: {
                if(keyc == keycode_Return || m_Terminators.contains(keyc)) {
                    emit lineInputRequestEnded(false, mp_LineInputBuffer, m_LineInputBufferPosition, m_Unicode);
                    Glk::sendTaskToGlkThread([&] {
                        Glk::Dispatch::unregisterArray(mp_LineInputBuffer, m_LineInputBufferLength, m_Unicode);
                    });
                    QGlk::getMainWindow().eventQueue().push(event_t {evtype_LineInput, TO_WINID(windowPointer()), m_LineInputBufferPosition, keyc == keycode_Return ? 0 : keyc});
                    m_LineInputRequested = false;
                }

                break;
            }
        }

        return true;
    }

    return false;
}

Glk::Window* Glk::KeyboardInputProvider::windowPointer() {
    return static_cast<Glk::Window*>(parent());
}

Glk::MouseInputProvider::MouseInputProvider(Glk::Window* parent_, bool m_MouseInputProvider) : QObject(parent_), m_MouseInputProvider(m_MouseInputProvider), m_MouseInputRequested(false) {
}

void Glk::MouseInputProvider::requestMouseInput() {
    if(!m_MouseInputProvider || m_MouseInputRequested)
        return;

    m_MouseInputRequested = true;

    emit mouseInputRequested();
}

void Glk::MouseInputProvider::cancelMouseInputRequest() {
    if(!m_MouseInputProvider || !m_MouseInputRequested)
        return;

    m_MouseInputRequested = false;

    emit mouseInputRequestEnded(true);
}

bool Glk::MouseInputProvider::handleMouseEvent(QMouseEvent* ev) { //TODO fix
    if(m_MouseInputRequested) {
        QSize ws = windowPointer()->windowSize();

        glui32 x = std::clamp(ev->pos().x(), 0, ws.width() - 1);
        glui32 y = std::clamp(ev->pos().y(), 0, ws.height() - 1);

        emit mouseInputRequestEnded(false);
        QGlk::getMainWindow().eventQueue().push(event_t {evtype_MouseInput, TO_WINID(windowPointer()), x, y});
        m_MouseInputRequested = false;
    }

    return false;
}

Glk::Window* Glk::MouseInputProvider::windowPointer() {
    return static_cast<Glk::Window*>(parent());
}

#include "moc_inputprovider.cpp"
