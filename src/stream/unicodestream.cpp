#include "unicodestream.hpp"

#include <cstring>

#include <QtEndian>

Glk::UnicodeStream::UnicodeStream(QObject* parent_, QIODevice* device_, Glk::Stream::Type type_, glui32 rock_) : Stream(parent_, device_, type_, true, rock_) {}

Glk::UnicodeStream::~UnicodeStream() {
}

glui32 Glk::UnicodeStream::position() const {
//     if(isInTextMode()) {
//         // TODO textmode
//     } else {
        return glui32(device()->pos() / 4);
//     }
}

void Glk::UnicodeStream::setPosition(glui32 pos) {
//     if(isInTextMode()) {
//         // TODO textmode
//     } else {
        device()->seek(pos * 4);
//     }
}

void Glk::UnicodeStream::writeBuffer(char* buf, glui32 len) {
    glui32* ibuf = new glui32[len];

    for(glui32 ii = 0; ii < len; ii++)
        ibuf[ii] = buf[ii];

    writeUnicodeBuffer(ibuf, len);

    delete[] ibuf;
}

void Glk::UnicodeStream::writeChar(unsigned char ch) {
    writeUnicodeChar(ch);
}

void Glk::UnicodeStream::writeString(char* str) {
    glui32 len = qstrlen(str);
    glui32* ibuf = new glui32[len];

    for(glui32 ii = 0; ii < len; ii++)
        ibuf[ii] = str[ii];

    writeUnicodeBuffer(ibuf, len);

    delete[] ibuf;
}

void Glk::UnicodeStream::writeUnicodeBuffer(glui32* buf, glui32 len) {
//     if(isInTextMode()) {
// 
//     } else {
        glui32* newbuf;

        if(!isInTextMode() && (!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource))) {
            newbuf = new glui32[len];
            std::memcpy(newbuf, buf, len);

            for(glui32 ii = 0; ii < len; ii++)
                newbuf[ii] = qToBigEndian(newbuf[ii]);
        } else {
            newbuf = buf;
        }

        qint64 writtenb = device()->write(reinterpret_cast<char*>(newbuf), len * 4);

        if(writtenb != -1)
            updateWriteCount(glui32(writtenb) / 4);

        if(!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource))
            delete[] newbuf;
//     }
}

void Glk::UnicodeStream::writeUnicodeChar(glui32 ch) {
//     if(isInTextMode()) {
// 
//     } else {
        if(!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource))
            ch = qToBigEndian(ch);

        qint64 writtenb = device()->write(reinterpret_cast<char*>(&ch), 4);

        if(writtenb != -1)
            updateWriteCount(1);
//     }
}

void Glk::UnicodeStream::writeUnicodeString(glui32* str) {
    glui32 len;

    for(len = 0; str[len] != 0; len++);

    writeUnicodeBuffer(str, len);
    return;
}

glui32 Glk::UnicodeStream::readBuffer(char* buf, glui32 len) {
    glui32* ibuf = new glui32[len];

    glui32 numr = readUnicodeBuffer(ibuf, len);

    for(glui32 ii = 0; ii < numr; ii++)
        buf[ii] = (ibuf[ii] >= 0x100 ? '?' : ibuf[ii]);

    delete[] ibuf;

    return numr;
}

glsi32 Glk::UnicodeStream::readChar() {
    glsi32 ch = readUnicodeChar();

    return (ch >= 0x100 ? '?' : ch);
}

glui32 Glk::UnicodeStream::readLine(char* buf, glui32 len) {
    glui32* ibuf = new glui32[len];

    glui32 numr = readUnicodeLine(ibuf, len);

    for(glui32 ii = 0; ii < numr + 1; ii++)
        buf[ii] = (ibuf[ii] >= 0x100 ? '?' : ibuf[ii]);

    delete[] ibuf;

    return numr;
}

glui32 Glk::UnicodeStream::readUnicodeBuffer(glui32* buf, glui32 len) {
//     if(isInTextMode()) {
// 
//     } else {
        qint64 numr = device()->read(reinterpret_cast<char*>(buf), len * 4);

        if(numr > 0)
            updateReadCount(glui32(numr) / 4);
        else
            return 0;

        if(!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource)) {
            for(glui32 ii = 0; ii < (numr) / 4; ii++)
                buf[ii] = qFromBigEndian(buf[ii]);
        }

        return glui32(numr) / 4;
//     }
}

glsi32 Glk::UnicodeStream::readUnicodeChar() {
//     if(isInTextMode()) {
// 
//     } else {
        glsi32 ch;

        if(device()->read(reinterpret_cast<char*>(&ch), 4) > 0) {
            updateReadCount(1);

            if(!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource))
                return qFromBigEndian(ch);
            else
                return ch;
        } else {
            return -1;
        }
//     }
}

glui32 Glk::UnicodeStream::readUnicodeLine(glui32* buf, glui32 len) {
//     if(isInTextMode()) {
// 
//     } else {
        glui32* iptr = buf - 1;
        glui32 numr = 0;

        do {
            iptr++;

            if(device()->read(reinterpret_cast<char*>(iptr), 4) > 0) {
                numr++;
            } else {
                numr++;
                break;
            }

            if(!isInTextMode() && (type() == Glk::Stream::Type::File || type() == Glk::Stream::Type::Resource))
                *iptr = qFromBigEndian(*iptr);
        } while(*iptr != 0);

        numr--;

        updateReadCount(numr);

        return numr;
//     }
}


