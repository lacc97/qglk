#include "unicodestream.hpp"

#include <QtEndian>

Glk::UnicodeStream::UnicodeStream(QIODevice* device_, Glk::Stream::Type type_, void* userptr_, glui32 rock_) : Stream(device_, type_, userptr_, rock_) {}

glui32 Glk::UnicodeStream::position() const {
    if(isInTextMode()) {
        // TODO textmode
    } else {
        return glui32(device()->pos() / 4);
    }
}

void Glk::UnicodeStream::setPosition(glui32 pos) {
    if(isInTextMode()) {
        // TODO textmode
    } else {
        device()->seek(pos * 4);
    }
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
    if(isInTextMode()) {
        
    } else {
        glui32* bufbe = new glui32[len];

        for(glui32 ii = 0; ii < len; ii++)
            bufbe[ii] = qToBigEndian(buf[ii]);

        qint64 writtenb = device()->write(reinterpret_cast<char*>(bufbe), len * 4);

        if(writtenb != -1)
            updateWriteCount(glui32(writtenb) / 4);
        
        delete[] bufbe;
    }
}

void Glk::UnicodeStream::writeUnicodeChar(glui32 ch) {
    if(isInTextMode()) {

    } else {
        glui32 chbe = qToBigEndian(ch);

        qint64 writtenb = device()->write(reinterpret_cast<char*>(&chbe), 4);

        if(writtenb != -1)
            updateWriteCount(1);
    }
}

void Glk::UnicodeStream::writeUnicodeString(glui32* str) {
    glui32 len;

    for(len = 0; str[len] != 0; len++);

    return writeUnicodeBuffer(str, len);
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
    if(isInTextMode()) {

    } else {
        qint64 numr = device()->read(reinterpret_cast<char*>(buf), len * 4);

        if(numr > 0)
            updateReadCount(glui32(numr) / 4);
        else
            return 0;

        for(glui32 ii = 0; ii < (numr) / 4; ii++)
            buf[ii] = qFromBigEndian(buf[ii]);

        return glui32(numr) / 4;
    }
}

glsi32 Glk::UnicodeStream::readUnicodeChar() {
    if(isInTextMode()) {

    } else {
        glsi32 ch;

        if(device()->read(reinterpret_cast<char*>(&ch), 4) > 0) {
            updateReadCount(1);
            return qFromBigEndian(ch);
        } else {
            return -1;
        }
    }
}

glui32 Glk::UnicodeStream::readUnicodeLine(glui32* buf, glui32 len) {
    if(isInTextMode()) {

    } else {
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

            *iptr = qFromBigEndian(*iptr);
        } while(*iptr != 0);

        numr--;

        updateReadCount(numr);

        return numr;
    }
}

