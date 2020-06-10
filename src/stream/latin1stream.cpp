#include "latin1stream.hpp"

#include <algorithm>
#include <memory>

#include <buffer/small_buffer.hpp>

Glk::Latin1Stream::Latin1Stream(QObject* parent_, QIODevice* device_, Glk::Stream::Type type_, glui32 rock_) : Stream(parent_, device_, type_, false, rock_) {}

Glk::Latin1Stream::~Latin1Stream() {
}

glui32 Glk::Latin1Stream::position() const {
    return glui32(device()->pos());
}

void Glk::Latin1Stream::setPosition(glui32 pos) {
    device()->seek(pos);
}


void Glk::Latin1Stream::writeBuffer(buffer::byte_buffer_view buf) {
    qint64 writtenb = device()->write(buf.data(), buf.size());

    if(writtenb != -1)
        updateWriteCount(glui32(writtenb));
}

void Glk::Latin1Stream::writeUnicodeBuffer(buffer::buffer_view<glui32> buf) {
    buffer::small_byte_buffer<BUFSIZ> cbuf{buf.size()};

    std::transform(buf.begin(), buf.end(), cbuf.begin(), [](glui32 ch) -> char {
        return (ch >= 0x100) ? '?' : char(ch);
    });

    writeBuffer(cbuf);
}

glui32 Glk::Latin1Stream::readBuffer(buffer::byte_buffer_span buf) {
    qint64 numr = device()->read(buf.data(), buf.size());
    if(numr <= 0)
        return 0;

    updateReadCount(glui32(numr));
    return glui32(numr);
}

glui32 Glk::Latin1Stream::readLine(buffer::byte_buffer_span buf) {
    qint64 numr = device()->readLine(buf.data(), buf.size());
    if(numr <= 0)
        return 0;

    updateReadCount(glui32(numr));
    return glui32(numr);
}

glui32 Glk::Latin1Stream::readUnicodeBuffer(buffer::buffer_span<glui32> buf) {
    buffer::small_byte_buffer<BUFSIZ> cbuf{buf.size()};

    glui32 numr = readBuffer(cbuf);
    std::transform(cbuf.begin(), cbuf.begin() + numr, buf.begin(), [](char ch) -> glui32 {
        return ch;
    });

    return numr;
}

glui32 Glk::Latin1Stream::readUnicodeLine(buffer::buffer_span<glui32> buf) {
    buffer::small_byte_buffer<BUFSIZ> cbuf{buf.size()};

    glui32 numr = readLine(cbuf);
    std::transform(cbuf.begin(), cbuf.begin() + numr, buf.begin(), [](char ch) -> glui32 {
        return ch;
    });

    return numr;
}
