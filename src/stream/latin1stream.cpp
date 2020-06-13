#include "latin1stream.hpp"

#include <algorithm>
#include <memory>

#include <buffer/small_buffer.hpp>

Glk::Latin1Stream::Latin1Stream(QObject* parent_, std::unique_ptr<std::streambuf> buf_, Type type_, bool text_, glui32 rock_)
        : Stream(parent_, std::move(buf_), type_, text_, false, rock_) {}

Glk::Latin1Stream::~Latin1Stream() = default;

glui32 Glk::Latin1Stream::position() const {
    return glui32(streambuf()->pubseekoff(0, std::ios_base::cur));
}

void Glk::Latin1Stream::setPosition(glsi32 off, std::ios_base::seekdir dir) {
    streambuf()->pubseekoff(off, dir);
}

void Glk::Latin1Stream::writeBuffer(buffer::byte_buffer_view buf) {
    updateWriteCount(streambuf()->sputn(buf.data(), buf.size()));
}

void Glk::Latin1Stream::writeUnicodeBuffer(buffer::buffer_view<glui32> buf) {
    buffer::small_byte_buffer<BUFSIZ> cbuf{buf.size()};

    std::transform(buf.begin(), buf.end(), cbuf.begin(), [](glui32 ch) -> char {
        return (ch >= 0x100) ? '?' : bit_cast<char>(static_cast<unsigned char>(ch));
    });

    writeBuffer(cbuf);
}

glui32 Glk::Latin1Stream::readBuffer(buffer::byte_buffer_span buf) {
    glui32 numr = streambuf()->sgetn(buf.data(), buf.size());
    updateReadCount(numr);
    return numr;
}

glui32 Glk::Latin1Stream::readLine(buffer::byte_buffer_span buf) {
    auto oldstrpos = streambuf()->pubseekoff(0, std::ios_base::cur);

    buffer::byte_buffer_span peekdata;
    {
        std::streamsize numr = streambuf()->sgetn(buf.data(),buf.size() - 1);
        if(numr <= 0)
            return 0;

        /* we work only with the valid data from the peek */
        peekdata = buf.first(numr);
    }

    glui32 readcount;
    {
        /* look for a newline */
        auto nlpos = std::find(peekdata.begin(), peekdata.end(), '\n');

        /* if we find a newline, we must count it */
        readcount = std::distance(peekdata.begin(), nlpos) + (nlpos != peekdata.end() ? 1 : 0);

        /* we don't care about anything after the newline */
        peekdata = peekdata.first(readcount);
    }

    /* null terminator isn't counted */
    buf[readcount] = 0;

    streambuf()->pubseekpos(oldstrpos + (decltype(oldstrpos))readcount);

    updateReadCount(readcount);
    return readcount;
}

glui32 Glk::Latin1Stream::readUnicodeBuffer(buffer::buffer_span<glui32> buf) {
    buffer::small_byte_buffer<BUFSIZ> cbuf{buf.size()};

    glui32 numr = readBuffer(cbuf);
    std::transform(cbuf.begin(), cbuf.begin() + numr, buf.begin(), [](char ch) -> glui32 {
        return bit_cast<unsigned char>(ch);
    });

    return numr;
}

glui32 Glk::Latin1Stream::readUnicodeLine(buffer::buffer_span<glui32> buf) {
    buffer::small_byte_buffer<BUFSIZ> cbuf{buf.size()};

    glui32 numr = readLine(cbuf);
    std::transform(cbuf.begin(), cbuf.begin() + numr, buf.begin(), [](char ch) -> glui32 {
        return bit_cast<unsigned char>(ch);
    });

    return numr;
}
