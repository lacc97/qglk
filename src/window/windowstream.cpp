#include "windowstream.hpp"

#include <cstring>

#include "window.hpp"

#include "thread/taskrequest.hpp"

Glk::WindowBuf::WindowBuf(Glk::Window* win)
    : mp_Window{win} {
    assert(mp_Window);
}

Glk::WindowBuf::int_type Glk::WindowBuf::overflow(int_type ch) {
    char c = traits_type::to_char_type(ch);
    if(xsputn(&c, 1) == 1)
        return traits_type::not_eof(ch);
    else
        return traits_type::eof();
}

std::streamsize Glk::WindowBuf::xsputn(const char* s, std::streamsize count) {
    return 0;
}

Glk::WindowStream::WindowStream(std::unique_ptr<WindowBuf> dev)
    : UnicodeStream(nullptr, std::move(dev), Stream::Type::Window, true, 0),
      mp_EchoStream{nullptr} {}

void Glk::WindowStream::setEchoStream(Glk::Stream* echo) {
    if(mp_EchoStream)
        QObject::disconnect(mp_EchoStream, nullptr, this, nullptr);

    mp_EchoStream = echo;

    if(mp_EchoStream)
        connect(mp_EchoStream, &Glk::Stream::closed,this, &Glk::WindowStream::onEchoStreamClosed,
                Qt::DirectConnection);
}

void Glk::WindowStream::pushStyle(Style::Type sty) {
    windowBuf()->window()->pushStyle(sty);
}

void Glk::WindowStream::pushHyperlink(glui32 linkValue) {
    windowBuf()->window()->pushHyperlink(linkValue);
}

void Glk::WindowStream::onEchoStreamClosed() {
    mp_EchoStream = nullptr;
}

void Glk::WindowStream::writeUnicodeBuffer(buffer::buffer_view<glui32> buf) {
    if(mp_EchoStream)
        mp_EchoStream->writeUnicodeBuffer(buf);

    UnicodeStream::writeUnicodeBuffer(buf);
}
