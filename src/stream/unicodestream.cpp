#include "unicodestream.hpp"

#include <cstring>

#include <buffer/small_buffer.hpp>
#include <buffer/static_buffer.hpp>

#include <QtEndian>

Glk::UnicodeStream::UnicodeStream(QObject* parent_, std::unique_ptr<std::streambuf> buf_, Type type_, bool text_, glui32 rock_)
        : Stream(parent_, std::move(buf_), type_, text_, true, rock_) {}

Glk::UnicodeStream::~UnicodeStream() = default;

bool Glk::UnicodeStream::isStreamBigEndian() const {
    return (!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource));
}

glui32 Glk::UnicodeStream::position() const {
//     if(isInTextMode()) {
//         // TODO textmode
//     } else {
    return glui32(streambuf()->pubseekoff(0, std::ios_base::cur) / sizeof(glui32));
//     }
}

void Glk::UnicodeStream::setPosition(glsi32 off, std::ios_base::seekdir dir) {
    streambuf()->pubseekoff(sizeof(glui32)*off, dir);
}

void Glk::UnicodeStream::writeBuffer(buffer::byte_buffer_view buf) {
    buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ibuf{buf.size()};

    if(isStreamBigEndian()) {
        std::transform(buf.begin(), buf.end(), ibuf.begin(), [](char ch) -> glui32 {
            return qToBigEndian<glui32>(ch);
        });
    } else {
        std::transform(buf.begin(), buf.end(), ibuf.begin(), [](char ch) -> glui32 {
            return ch;
        });
    }

    updateWriteCount(glui32(streambuf()->sputn((char*)ibuf.data(), sizeof(glui32) * ibuf.size()) / sizeof(glui32)));
}

void Glk::UnicodeStream::writeUnicodeBuffer(buffer::buffer_view<glui32> buf) {
    buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ibuf{buf.size()};

    buffer::buffer_view<glui32> writebuf = buf;
    if(isStreamBigEndian()) {
        std::transform(buf.begin(), buf.end(), ibuf.begin(), [](char ch) -> glui32 {
            return qToBigEndian<glui32>(ch);
        });
        writebuf = ibuf;
    }

    updateWriteCount(glui32(streambuf()->sputn((char*)writebuf.data(), sizeof(glui32) * writebuf.size()) / sizeof(glui32)));
}

glui32 Glk::UnicodeStream::readBuffer(buffer::byte_buffer_span buf) {
    buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ibuf{buf.size()};

    glui32 numr = readUnicodeBuffer(ibuf);
    std::transform(ibuf.begin(), ibuf.begin() + numr, buf.begin(), [](glui32 ch) -> char {
        return (ch >= 0x100) ? '?' : char(ch);
    });

    return numr;
}

glui32 Glk::UnicodeStream::readLine(buffer::byte_buffer_span buf) {
    buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ibuf{buf.size()};

    glui32 numr = readUnicodeLine(ibuf);
    std::transform(ibuf.begin(), ibuf.begin() + numr, buf.begin(), [](glui32 ch) -> char {
        return (ch >= 0x100) ? '?' : char(ch);
    });

    return numr;
}

glui32 Glk::UnicodeStream::readUnicodeBuffer(buffer::buffer_span<glui32> buf) {
    //     if(isInTextMode()) {
//
//     } else {
    glui32 readcount = streambuf()->sgetn((char*)buf.data(), sizeof(glui32)*buf.size()) / sizeof(glui32);

    if(isStreamBigEndian()) {
        buf = buf.subspan(readcount);
        std::transform(buf.begin(), buf.end(), buf.begin(), [](glui32 ch) -> glui32 {
            return qFromBigEndian<glui32>(ch);
        });
    }

    updateReadCount(readcount);
    return readcount;
//     }
}

glui32 Glk::UnicodeStream::readUnicodeLine(buffer::buffer_span<glui32> buf) {
//     if(isInTextMode()) {
//
//     } else {
    auto oldstrpos = streambuf()->pubseekoff(0, std::ios_base::cur);

    buffer::buffer_span<glui32> peekdata;
    {
        std::streamsize numr = streambuf()->sgetn((char*)buf.data(), sizeof(glui32) * (buf.size() - 1));
        if(numr <= 0)
            return 0;

        /* we work only with the valid data from the peek */
        peekdata = buf.first(glui32(numr) / sizeof(glui32));
    }

    glui32 readcount;
    {
        /* look for a newline */
        glui32 nl = isStreamBigEndian() ? qToBigEndian<glui32>('\n') : '\n';
        auto nlpos = std::find(peekdata.begin(), peekdata.end(), nl);

        /* if we find a newline, we must count it */
        readcount = std::distance(peekdata.begin(), nlpos) + (nlpos != peekdata.end() ? 1 : 0);

        /* we don't care about anything after the newline */
        peekdata = peekdata.first(readcount);
    }

    /* convert from big endian if necessary */
    if(isStreamBigEndian()) {
        std::transform(peekdata.begin(), peekdata.end(), peekdata.begin(), [](glui32 ch) -> glui32 {
            return qFromBigEndian<glui32>(ch);
        });
    }

    /* null terminator isn't counted */
    buf[readcount] = 0;

    streambuf()->pubseekpos(oldstrpos + (decltype(oldstrpos))(readcount*sizeof(glui32)));

    updateReadCount(readcount);
    return readcount;
//     }
}

