#include "windowstream.hpp"

#include "window.hpp"

#include "thread/taskrequest.hpp"

Glk::WindowStream::WindowStream(Window* parent_, QIODevice* device_) : UnicodeStream(parent_, device_, Stream::Type::Window), mp_EchoStream(NULL) {}

void Glk::WindowStream::setEchoStream(Glk::Stream* echo) {
    if(mp_EchoStream)
        QObject::disconnect(mp_EchoStream, 0, this, 0);

    mp_EchoStream = echo;

    if(mp_EchoStream)
        connect(mp_EchoStream, &Glk::Stream::closed, this, &Glk::WindowStream::onEchoStreamClosed, Qt::DirectConnection);
}

void Glk::WindowStream::pushStyle(Style::Type sty) {
    static_cast<Window*>(parent())->setStyle(sty);
}

void Glk::WindowStream::writeUnicodeBuffer(glui32* buf, glui32 len) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeBuffer(buf, len);

    Glk::sendTaskToEventThread([&] {
        UnicodeStream::writeUnicodeBuffer(buf, len);
    });
}

void Glk::WindowStream::writeUnicodeChar(glui32 ch) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeChar(ch);

    Glk::sendTaskToEventThread([&] {
        UnicodeStream::writeUnicodeChar(ch);
    });
}

void Glk::WindowStream::writeUnicodeString(glui32* str) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeString(str);

    Glk::sendTaskToEventThread([&] {
        UnicodeStream::writeUnicodeString(str);
    });
}

void Glk::WindowStream::onEchoStreamClosed() {
    mp_EchoStream = NULL;
}

#include "moc_windowstream.cpp"
