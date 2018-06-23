#include "latin1stream.hpp"

Glk::Latin1Stream::Latin1Stream(QObject* parent_, QIODevice* device_, Glk::Stream::Type type_, void* userptr_, glui32 rock_) : Stream(parent_, device_, type_, userptr_, rock_) {}

Glk::Latin1Stream::~Latin1Stream() {
    close();
}

glui32 Glk::Latin1Stream::position() const {
    return glui32(device()->pos());
}

void Glk::Latin1Stream::setPosition(glui32 pos) {
    device()->seek(pos);
}

void Glk::Latin1Stream::writeBuffer(char* buf, glui32 len) {
    qint64 writtenb = device()->write(buf, len);

    if(writtenb != -1)
        updateWriteCount(glui32(writtenb));
}

void Glk::Latin1Stream::writeChar(unsigned char ch) {
    if(device()->putChar(*reinterpret_cast<char*>(&ch)))
        updateWriteCount(1);
}

void Glk::Latin1Stream::writeString(char* str) {
    qint64 writtenb = device()->write(str);

    if(writtenb != -1)
        updateWriteCount(glui32(writtenb));
}

void Glk::Latin1Stream::writeUnicodeBuffer(glui32* buf, glui32 len) {
    char* cbuf = new char[len];

    for(glui32 ii = 0; ii < len; ii++)
        cbuf[ii] = (buf[ii] >= 0x100 ? '?' : char(buf[ii]));

    writeBuffer(cbuf, len);

    delete[] cbuf;
}

void Glk::Latin1Stream::writeUnicodeChar(glui32 ch) {
    char c = (ch >= 0x100 ? '?' : char(ch));
    writeChar(c);
}

void Glk::Latin1Stream::writeUnicodeString(glui32* str) {
    glui32 len;

    for(len = 0; str[len] != 0; len++);

    char* cbuf = new char[len];

    for(glui32 ii = 0; ii < len; ii++)
        cbuf[ii] = (str[ii] >= 0x100 ? '?' : char(str[ii]));

    writeBuffer(cbuf, len);

    delete[] cbuf;
}

glui32 Glk::Latin1Stream::readBuffer(char* buf, glui32 len) {
    qint64 numr = device()->read(buf, len);

    if(numr > 0)
        updateReadCount(glui32(numr));
    else
        return 0;

    return glui32(numr);
}

glsi32 Glk::Latin1Stream::readChar() {
    char c;

    if(device()->getChar(&c)) {
        updateReadCount(1);
        return c;
    } else {
        return -1;
    }
}

glui32 Glk::Latin1Stream::readLine(char* buf, glui32 len) {
    qint64 numr = device()->readLine(buf, len);

    if(numr > 0)
        updateReadCount(glui32(numr));
    else
        return 0;

    return glui32(numr);
}

glui32 Glk::Latin1Stream::readUnicodeBuffer(glui32* buf, glui32 len) {
    char* cbuf = new char[len];

    glui32 numr = readBuffer(cbuf, len);

    for(glui32 ii = 0; ii < numr; ii++)
        buf[ii] = cbuf[ii];

    return numr;
}

glsi32 Glk::Latin1Stream::readUnicodeChar() {
    return readChar();
}

glui32 Glk::Latin1Stream::readUnicodeLine(glui32* buf, glui32 len) {
    char* cbuf = new char[len];

    glui32 numr = readLine(cbuf, len);

    for(glui32 ii = 0; ii < numr + 1; ii++)
        buf[ii] = cbuf[ii];

    return numr;
}
