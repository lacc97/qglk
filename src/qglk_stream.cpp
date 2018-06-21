#include "file/fileref.hpp"

#include "blorb/chunk.hpp"

#include "stream/latin1stream.hpp"
#include "stream/nulldevice.hpp"
#include "stream/unicodestream.hpp"

#include <QBuffer>
#include <QSet>

QSet<Glk::Stream*> s_StreamSet;

void glk_stream_close(strid_t str, stream_result_t* result) {
    if(result) {
        result->readcount = FROM_STRID(str)->readCount();
        result->writecount = FROM_STRID(str)->writeCount();
    }

    delete FROM_STRID(str);
}

strid_t glk_stream_iterate(strid_t str, glui32* rockptr) {
    if(str == NULL) {
        auto iter = s_StreamSet.begin();

        if(rockptr)
            *rockptr = (*iter)->rock();

        return TO_STRID(*iter);
    }

    auto iter = s_StreamSet.find(FROM_STRID(str));
    iter++;

    if(iter != s_StreamSet.end()) {
        if(rockptr)
            *rockptr = (*iter)->rock();

        return TO_STRID(*iter);
    } else
        return NULL;
}

glui32 glk_stream_get_rock(strid_t str) {
    return FROM_STRID(str)->rock();
}

void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode) {
    switch(seekmode) {
        case seekmode_Start:
            FROM_STRID(str)->setPosition(pos);
            return;

        case seekmode_End:
            FROM_STRID(str)->setPosition(pos);
            return;

        case seekmode_Current:
            FROM_STRID(str)->setPosition(FROM_STRID(str)->position() + pos);
            return;
    }
}

glui32 glk_stream_get_position(strid_t str) {
    return FROM_STRID(str)->position();
}

Glk::Stream* s_CurrentStream = NULL;
void glk_stream_set_current(strid_t str) {
    s_CurrentStream = FROM_STRID(str);
}

strid_t glk_stream_get_current(void) {
    return TO_STRID(s_CurrentStream);
}

void glk_put_char(unsigned char ch) {
    if(s_CurrentStream)
        s_CurrentStream->writeChar(ch);
}

void glk_put_char_stream(strid_t str, unsigned char ch) {
    FROM_STRID(str)->writeChar(ch);
}

void glk_put_string(char* s) {
    if(s_CurrentStream)
        s_CurrentStream->writeString(s);
}

void glk_put_string_stream(strid_t str, char* s) {
    FROM_STRID(str)->writeString(s);
}

void glk_put_buffer(char* buf, glui32 len) {
    if(s_CurrentStream)
        s_CurrentStream->writeBuffer(buf, len);
}

void glk_put_buffer_stream(strid_t str, char* buf, glui32 len) {
    FROM_STRID(str)->writeBuffer(buf, len);
}

void glk_set_style(glui32 styl) {
    if(s_CurrentStream)
        s_CurrentStream->pushStyle(static_cast<Glk::Style::Type>(styl));
}

void glk_set_style_stream(strid_t str, glui32 styl) {
    FROM_STRID(str)->pushStyle(static_cast<Glk::Style::Type>(styl));
}

glsi32 glk_get_char_stream(strid_t str) {
    return FROM_STRID(str)->readChar();
}

glui32 glk_get_line_stream(strid_t str, char* buf, glui32 len) {
    return FROM_STRID(str)->readLine(buf, len);
}

glui32 glk_get_buffer_stream(strid_t str, char* buf, glui32 len) {
    return FROM_STRID(str)->readBuffer(buf, len);
}

void glk_put_char_uni(glui32 ch) {
    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeChar(ch);
}

void glk_put_string_uni(glui32* s) {
    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeString(s);
}

void glk_put_buffer_uni(glui32* buf, glui32 len) {
    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeBuffer(buf, len);
}

void glk_put_char_stream_uni(strid_t str, glui32 ch) {
    FROM_STRID(str)->writeUnicodeChar(ch);
}

void glk_put_string_stream_uni(strid_t str, glui32* s) {
    FROM_STRID(str)->writeUnicodeString(s);
}

void glk_put_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    FROM_STRID(str)->writeUnicodeBuffer(buf, len);
}

glsi32 glk_get_char_stream_uni(strid_t str) {
    return FROM_STRID(str)->readUnicodeChar();
}

glui32 glk_get_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    return FROM_STRID(str)->readUnicodeBuffer(buf, len);
}

glui32 glk_get_line_stream_uni(strid_t str, glui32* buf, glui32 len) {
    return FROM_STRID(str)->readUnicodeLine(buf, len);
}

strid_t glk_stream_open_memory(char* buf, glui32 buflen, glui32 fmode, glui32 rock) {
    Glk::Stream* str;

    if(buf)
        str = new Glk::Latin1Stream(NULL, new QBuffer(new QByteArray(QByteArray::fromRawData(buf, buflen))), Glk::Stream::Type::Memory, buf, rock);
    else
        str = new Glk::Latin1Stream(NULL, new Glk::NullDevice(), Glk::Stream::Type::Memory, buf, rock);

    QIODevice::OpenMode om;

    switch(fmode) {
        case filemode_Write:
            om = QIODevice::WriteOnly;
            break;

        case filemode_Read:
            om = QIODevice::ReadOnly;
            break;

        case filemode_ReadWrite:
            om = QIODevice::ReadWrite;
            break;

        case filemode_WriteAppend:
            om = QIODevice::WriteOnly | QIODevice::Append;
            break;
    }

    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    if(buf)
        Glk::Dispatch::registerArray(buf, buflen, false);

    return TO_STRID(str);
}

strid_t glk_stream_open_memory_uni(glui32* buf, glui32 buflen, glui32 fmode, glui32 rock) {
    Glk::Stream* str;

    if(buf)
        str = new Glk::UnicodeStream(NULL, new QBuffer(new QByteArray(QByteArray::fromRawData(reinterpret_cast<char*>(buf), 4 * buflen))), Glk::Stream::Type::Memory, buf, rock);
    else
        str = new Glk::UnicodeStream(NULL, new Glk::NullDevice(), Glk::Stream::Type::Memory, buf, rock);

    QIODevice::OpenMode om;

    switch(fmode) {
        case filemode_Write:
            om = QIODevice::WriteOnly;
            break;

        case filemode_Read:
            om = QIODevice::ReadOnly;
            break;

        case filemode_ReadWrite:
            om = QIODevice::ReadWrite;
            break;

        case filemode_WriteAppend:
            om = QIODevice::WriteOnly | QIODevice::Append;
            break;
    }

    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    if(buf)
        Glk::Dispatch::registerArray(buf, buflen, true);

    return TO_STRID(str);
}

strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock) {
    Glk::Stream* str = new Glk::Latin1Stream(NULL, FROM_FREFID(fileref)->file(), Glk::Stream::Type::File, NULL, rock);
    QIODevice::OpenMode om;

    switch(fmode) {
        case filemode_Write:
            om = QIODevice::WriteOnly;
            break;

        case filemode_Read:
            om = QIODevice::ReadOnly;
            break;

        case filemode_ReadWrite:
            om = QIODevice::ReadWrite;
            break;

        case filemode_WriteAppend:
            om = QIODevice::WriteOnly | QIODevice::Append;
            break;
    }

    switch(FROM_FREFID(fileref)->usage() & 0x100) {
        case Glk::FileReference::TextMode:
            om |= QIODevice::Text;
            break;
    }

    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    return TO_STRID(str);
}

strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock) {
    Glk::Stream* str = new Glk::UnicodeStream(NULL, FROM_FREFID(fileref)->file(), Glk::Stream::Type::File, NULL, rock);
    QIODevice::OpenMode om;

    switch(fmode) {
        case filemode_Write:
            om = QIODevice::WriteOnly;
            break;

        case filemode_Read:
            om = QIODevice::ReadOnly;
            break;

        case filemode_ReadWrite:
            om = QIODevice::ReadWrite;
            break;

        case filemode_WriteAppend:
            om = QIODevice::WriteOnly | QIODevice::Append;
            break;
    }

    switch(FROM_FREFID(fileref)->usage() & 0x100) {
        case Glk::FileReference::TextMode:
            om |= QIODevice::Text;
            break;
    }

    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    return TO_STRID(str);
}

strid_t glk_stream_open_resource(glui32 filenum, glui32 rock) {
    Glk::Blorb::Chunk* chunk;

    if(!(chunk = new Glk::Blorb::Chunk(Glk::Blorb::loadResource(filenum)))->isValid()) {
        delete chunk;
        return NULL;
    }

    Glk::Stream* str = new Glk::Latin1Stream(NULL, new QBuffer(new QByteArray(QByteArray::fromRawData(chunk->data(), chunk->length()))), Glk::Stream::Type::Resource, chunk, rock);
    QIODevice::OpenMode om = QIODevice::ReadOnly;

    if(chunk->type() == Glk::Blorb::ChunkType::TEXT)
        om |= QIODevice::Text;

    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    return TO_STRID(str);
}

strid_t glk_stream_open_resource_uni(glui32 filenum, glui32 rock) {
    Glk::Blorb::Chunk* chunk;

    if(!(chunk = new Glk::Blorb::Chunk(Glk::Blorb::loadResource(filenum)))->isValid()) {
        delete chunk;
        return NULL;
    }

    Glk::Stream* str = new Glk::UnicodeStream(NULL, new QBuffer(new QByteArray(QByteArray::fromRawData(chunk->data(), chunk->length()))), Glk::Stream::Type::Resource, chunk, rock);
    QIODevice::OpenMode om = QIODevice::ReadOnly;

    if(chunk->type() == Glk::Blorb::ChunkType::TEXT)
        om |= QIODevice::Text;

    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    return TO_STRID(str);
}

void glk_set_hyperlink(glui32 linkval) {

}

void glk_set_hyperlink_stream(strid_t str, glui32 linkval) {

}
