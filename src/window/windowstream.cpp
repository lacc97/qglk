#include "windowstream.hpp"

#include <cstring>

#include "window.hpp"

#include "thread/taskrequest.hpp"

Glk::WindowDevice::WindowDevice(Glk::Window* win)
    : QIODevice(nullptr),
      mp_Window{win} {
    assert(mp_Window);
}

qint64 Glk::WindowDevice::readData(char* data, qint64 maxlen) {
    std::memset(data, 0, maxlen);
    return maxlen;
}

qint64 Glk::WindowDevice::writeData(const char* data, qint64 len) {
    return 0;
}

Glk::WindowStream::WindowStream(WindowDevice* dev)
    : UnicodeStream(nullptr, dev, Stream::Type::Window),
      mp_EchoStream{nullptr} {}

void Glk::WindowStream::setEchoStream(Glk::Stream* echo) {
    if(mp_EchoStream)
        QObject::disconnect(mp_EchoStream, nullptr, this, nullptr);

    mp_EchoStream = echo;

    if(mp_EchoStream)
        connect(mp_EchoStream, &Glk::Stream::closed, this, &Glk::WindowStream::onEchoStreamClosed,
                Qt::DirectConnection);
}

void Glk::WindowStream::pushStyle(Style::Type sty) {
    windowDevice()->window()->pushStyle(sty);
}

void Glk::WindowStream::pushHyperlink(glui32 linkValue) {
    windowDevice()->window()->pushHyperlink(linkValue);
}

void Glk::WindowStream::writeUnicodeBuffer(glui32* buf, glui32 len) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeBuffer(buf, len);

    UnicodeStream::writeUnicodeBuffer(buf, len);
}

void Glk::WindowStream::writeUnicodeChar(glui32 ch) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeChar(ch);

    UnicodeStream::writeUnicodeChar(ch);
}

void Glk::WindowStream::writeUnicodeString(glui32* str) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeString(str);

    UnicodeStream::writeUnicodeString(str);
}

void Glk::WindowStream::onEchoStreamClosed() {
    mp_EchoStream = nullptr;
}
