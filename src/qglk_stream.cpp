#include "file/fileref.hpp"

#include <cstring>

#include <QBuffer>
#include <QDebug>

#include "qglk.hpp"

#include "blorb/chunk.hpp"
#include "stream/latin1stream.hpp"
#include "stream/nulldevice.hpp"
#include "stream/unicodestream.hpp"

Glk::Stream* s_CurrentStream = NULL;
void glk_stream_set_current(strid_t str) {
#ifndef NDEBUG
    qDebug() << "glk_stream_set_current(" << str << ")";
#endif
    s_CurrentStream = FROM_STRID(str);
}

strid_t glk_stream_get_current(void) {
#ifndef NDEBUG
    qDebug() << "glk_stream_get_current() =>" << TO_STRID(s_CurrentStream);
#endif
    return TO_STRID(s_CurrentStream);
}

void glk_stream_close(strid_t str, stream_result_t* result) {
#ifndef NDEBUG
    qDebug() << "glk_stream_close(" << str << "," << result << ")";
#endif

    if(result) {
        result->readcount = FROM_STRID(str)->readCount();
        result->writecount = FROM_STRID(str)->writeCount();
    }

    if(TO_STRID(s_CurrentStream) == str)
        s_CurrentStream = NULL;

    delete FROM_STRID(str);
}

strid_t glk_stream_iterate(strid_t str, glui32* rockptr) {
    if(str == NULL) {
        if(QGlk::getMainWindow().streamList().empty())
            return NULL;

        Glk::Stream* first = QGlk::getMainWindow().streamList().first();

        if(rockptr)
            *rockptr = first->rock();

        return TO_STRID(first);
    }

    auto it = QGlk::getMainWindow().streamList().cbegin();

    while(it != QGlk::getMainWindow().streamList().cend() && (*it++) != FROM_STRID(str));

    if(it == QGlk::getMainWindow().streamList().cend())
        return NULL;

    if(rockptr)
        *rockptr = (*it)->rock();

    return TO_STRID(*it);
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
            FROM_STRID(str)->setPosition(FROM_STRID(str)->size() - pos);
            return;

        case seekmode_Current:
            FROM_STRID(str)->setPosition(FROM_STRID(str)->position() + pos);
            return;
    }
}

glui32 glk_stream_get_position(strid_t str) {
    return FROM_STRID(str)->position();
}

void glk_put_char(unsigned char ch) {
#ifndef NDEBUG
    qDebug() << "glk_put_char(" << QString(ch) << ")";
#endif

    if(s_CurrentStream)
        s_CurrentStream->writeChar(ch);
}

void glk_put_char_stream(strid_t str, unsigned char ch) {
#ifndef NDEBUG
    qDebug() << "glk_put_char_stream(" << str << "," << QString(ch) << ")";
#endif

    FROM_STRID(str)->writeChar(ch);
}

void glk_put_string(char* s) {
#ifndef NDEBUG
    qDebug() << "glk_put_string(" << QString(s) << ")";
#endif

    if(s_CurrentStream)
        s_CurrentStream->writeString(s);
}

void glk_put_string_stream(strid_t str, char* s) {
#ifndef NDEBUG
    qDebug() << "glk_put_string_stream(" << str << "," << QString(s) << ")";
#endif

    FROM_STRID(str)->writeString(s);
}

void glk_put_buffer(char* buf, glui32 len) {
#ifndef NDEBUG
    qDebug() << "glk_put_buffer(" << QString::fromLatin1(buf, len) << ")";
#endif

    if(s_CurrentStream)
        s_CurrentStream->writeBuffer(buf, len);
}

void glk_put_buffer_stream(strid_t str, char* buf, glui32 len) {
#ifndef NDEBUG
    qDebug() << "glk_put_buffer_stream(" << str << "," << QString::fromLatin1(buf, len) << ")";
#endif

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
    glsi32 ch = FROM_STRID(str)->readChar();
#ifndef NDEBUG
    qDebug() << "glk_get_char_stream(" << str << ") =>" << QString::fromUcs4(reinterpret_cast<glui32*>(&ch), 1);
#endif
    return ch;
}

glui32 glk_get_line_stream(strid_t str, char* buf, glui32 len) {
    return FROM_STRID(str)->readLine(buf, len);
}

glui32 glk_get_buffer_stream(strid_t str, char* buf, glui32 len) {
    return FROM_STRID(str)->readBuffer(buf, len);
}

void glk_put_char_uni(glui32 ch) {
#ifndef NDEBUG
    qDebug() << "glk_put_char_uni(" << QString::fromUcs4(&ch, 1) << ")";
#endif

    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeChar(ch);
}

void glk_put_string_uni(glui32* s) {
#ifndef NDEBUG
    qDebug() << "glk_put_string_uni(" << QString::fromUcs4(s) << ")";
#endif

    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeString(s);
}

void glk_put_buffer_uni(glui32* buf, glui32 len) {
#ifndef NDEBUG
    qDebug() << "glk_put_buffer_uni(" << QString::fromUcs4(buf, len) << ")";
#endif

    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeBuffer(buf, len);
}

void glk_put_char_stream_uni(strid_t str, glui32 ch) {
#ifndef NDEBUG
    qDebug() << "glk_put_char_stream_uni(" << str << "," << QString::fromUcs4(&ch, 1) << ")";
#endif

    FROM_STRID(str)->writeUnicodeChar(ch);
}

void glk_put_string_stream_uni(strid_t str, glui32* s) {
#ifndef NDEBUG
    qDebug() << "glk_put_string_stream_uni(" << str << "," << QString::fromUcs4(s) << ")";
#endif

    FROM_STRID(str)->writeUnicodeString(s);
}

void glk_put_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
#ifndef NDEBUG
    qDebug() << "glk_put_buffer_stream_uni(" << str << "," << QString::fromUcs4(buf, len) << ")";
#endif

    FROM_STRID(str)->writeUnicodeBuffer(buf, len);
}

glsi32 glk_get_char_stream_uni(strid_t str) {
    glsi32 ch = FROM_STRID(str)->readUnicodeChar();
#ifndef NDEBUG
    qDebug() << "glk_get_char_stream_uni(" << str << ") =>" << QString::fromUcs4(reinterpret_cast<glui32*>(&ch), 1);
#endif
    return ch;
}

glui32 glk_get_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    return FROM_STRID(str)->readUnicodeBuffer(buf, len);
}

glui32 glk_get_line_stream_uni(strid_t str, glui32* buf, glui32 len) {
    return FROM_STRID(str)->readUnicodeLine(buf, len);
}

strid_t glk_stream_open_memory(char* buf, glui32 buflen, glui32 fmode, glui32 rock) {
#ifndef NDEBUG
    QDebug deb = qDebug();
    deb << "glk_stream_open_memory(" << ((void*)buf) << "," << buflen << "," << fmode << "," << rock << ") =>";
#endif

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

    Glk::Stream* str;

    if(buf) {
        QByteArray* qba = new QByteArray(QByteArray::fromRawData(buf, buflen));
        str = new Glk::Latin1Stream(NULL, new QBuffer(qba), Glk::Stream::Type::Memory, rock);

        QObject::connect(str, &QObject::destroyed, [buf, buflen, om, qba]() {
            if((om & QIODevice::WriteOnly) != 0)
                std::memcpy(buf, qba->data(), buflen);

            Glk::Dispatch::unregisterArray(buf, buflen, false);

            delete qba;
        });
    } else {
        str = new Glk::Latin1Stream(NULL, new Glk::NullDevice(), Glk::Stream::Type::Memory, rock);
    }

    if(!str->open(om)) {
        delete str;
#ifndef NDEBUG
        deb << ((void*)NULL);
#endif
        return NULL;
    }

    if(buf)
        Glk::Dispatch::registerArray(buf, buflen, false);

#ifndef NDEBUG
    deb << TO_STRID(str);
#endif

    return TO_STRID(str);
}

strid_t glk_stream_open_memory_uni(glui32* buf, glui32 buflen, glui32 fmode, glui32 rock) {
#ifndef NDEBUG
    QDebug deb = qDebug();
    deb << "glk_stream_open_memory_uni(" << ((void*)buf) << "," << buflen << "," << fmode << "," << rock << ") =>";
#endif

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

    Glk::Stream* str;

    if(buf) {
        QByteArray* qba = new QByteArray(QByteArray::fromRawData(reinterpret_cast<char*>(buf), 4 * buflen));
        str = new Glk::Latin1Stream(NULL, new QBuffer(qba), Glk::Stream::Type::Memory, rock);

        QObject::connect(str, &QObject::destroyed, [buf, buflen, om, qba]() {
            if((om & QIODevice::WriteOnly) != 0)
                std::memcpy(buf, qba->data(), buflen);

            Glk::Dispatch::unregisterArray(buf, buflen, true);

            delete qba;
        });
    } else {
        str = new Glk::Latin1Stream(NULL, new Glk::NullDevice(), Glk::Stream::Type::Memory, rock);
    }

    if(!str->open(om)) {
        delete str;
#ifndef NDEBUG
        deb << ((void*)NULL);
#endif
        return NULL;
    }

    if(buf)
        Glk::Dispatch::registerArray(buf, buflen, true);

#ifndef NDEBUG
    deb << TO_STRID(str);
#endif

    return TO_STRID(str);
}

strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock) {
#ifndef NDEBUG
    QDebug deb = qDebug();
    deb << "glk_stream_open_file(" << FROM_FREFID(fileref)->path() << "," << fmode << "," << rock << ") =>";
#endif
    Glk::Stream* str = new Glk::Latin1Stream(NULL, FROM_FREFID(fileref)->file(), Glk::Stream::Type::File, rock);
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
#ifndef NDEBUG
        deb << ((void*)NULL);
#endif
        return NULL;
    }

#ifndef NDEBUG
    deb << TO_STRID(str);
#endif

    return TO_STRID(str);
}

strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock) {
#ifndef NDEBUG
    QDebug deb = qDebug();
    deb << "glk_stream_open_file_uni(" << FROM_FREFID(fileref)->path() << "," << fmode << "," << rock << ") =>";
#endif
    Glk::Stream* str = new Glk::UnicodeStream(NULL, FROM_FREFID(fileref)->file(), Glk::Stream::Type::File, rock);
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
#ifndef NDEBUG
        deb << ((void*)NULL);
#endif
        return NULL;
    }

#ifndef NDEBUG
    deb << TO_STRID(str);
#endif

    return TO_STRID(str);
}

strid_t glk_stream_open_resource(glui32 filenum, glui32 rock) {
    Glk::Blorb::Chunk* chunk;

    if(!(chunk = new Glk::Blorb::Chunk(Glk::Blorb::loadResource(filenum)))->isValid()) {
        delete chunk;
        return NULL;
    }

    QByteArray* qba = new QByteArray(QByteArray::fromRawData(chunk->data(), chunk->length()));
    Glk::Stream* str = new Glk::Latin1Stream(NULL, new QBuffer(qba), Glk::Stream::Type::Resource, rock);

    QObject::connect(str, &QObject::destroyed, [qba, chunk]() {
        delete qba;

        Glk::Blorb::unloadChunk(*chunk);
        delete chunk;
    });

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

    QByteArray* qba = new QByteArray(QByteArray::fromRawData(chunk->data(), chunk->length()));
    Glk::Stream* str = new Glk::UnicodeStream(NULL, new QBuffer(qba), Glk::Stream::Type::Resource, rock);

    QObject::connect(str, &QObject::destroyed, [qba, chunk]() {
        delete qba;

        Glk::Blorb::unloadChunk(*chunk);
        delete chunk;
    });

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
