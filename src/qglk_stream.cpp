#include "file/fileref.hpp"

#include <cstring>

#include <filesystem>
#include <fstream>
#include <memory>

#include <buffer/small_buffer.hpp>

#include "qglk.hpp"

#include "blorb/chunk.hpp"
#include "log/log.hpp"
#include "stream/chunkbuf.hpp"
#include "stream/latin1stream.hpp"
#include "stream/membuf.hpp"
#include "stream/nullbuf.hpp"
#include "stream/unicodestream.hpp"

Glk::Stream* s_CurrentStream = NULL;
void glk_stream_set_current(strid_t str) {
    SPDLOG_TRACE("glk_stream_set_current({})", wrap::ptr(str));

    s_CurrentStream = FROM_STRID(str);
}

strid_t glk_stream_get_current(void) {
    SPDLOG_TRACE("glk_stream_get_current() => {}", wrap::ptr(s_CurrentStream));

    return TO_STRID(s_CurrentStream);
}

void glk_stream_close(strid_t str, stream_result_t* result) {
    SPDLOG_TRACE("glk_stream_close({}, {})", wrap::ptr(str), (void*)(result));

    if(result) {
        result->readcount = FROM_STRID(str)->readCount();
        result->writecount = FROM_STRID(str)->writeCount();
    }

    if(TO_STRID(s_CurrentStream) == str)
        s_CurrentStream = NULL;

    delete FROM_STRID(str);
}

strid_t glk_stream_iterate(strid_t str, glui32* rockptr) {
    const auto& strList = QGlk::getMainWindow().streamList();

    if(str == NULL) {
        if(strList.empty()) {
            SPDLOG_TRACE("glk_stream_iterate({}, {}) => {}", wrap::ptr(str), wrap::ptr(rockptr), wrap::ptr(strid_t(NULL)));
            return NULL;
        }

        auto first = strList.front();

        if(rockptr)
            *rockptr = first->rock();

        SPDLOG_TRACE("glk_stream_iterate({}, {}) => {}", wrap::ptr(str), wrap::ptr(rockptr), wrap::ptr(first));

        return TO_STRID(first);
    }

    auto it = strList.cbegin();

    while(it != strList.cend() && (*it++) != FROM_STRID(str));

    if(it == strList.cend()) {
        SPDLOG_TRACE("glk_stream_iterate({}, {}) => {}", wrap::ptr(str), wrap::ptr(rockptr), wrap::ptr(strid_t(NULL)));
        return NULL;
    }

    if(rockptr)
        *rockptr = (*it)->rock();

    SPDLOG_TRACE("glk_stream_iterate({}, {}) => {}", wrap::ptr(str), wrap::ptr(rockptr), wrap::ptr(*it));

    return TO_STRID(*it);
}

glui32 glk_stream_get_rock(strid_t str) {
    return FROM_STRID(str)->rock();
}

void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode) {
    SPDLOG_TRACE("glk_stream_set_position({}, {}, {})", wrap::ptr(str), pos, wrap::seekmode(seekmode));

    switch(seekmode) {
        case seekmode_Start:
            FROM_STRID(str)->setPosition(pos, std::ios_base::beg);
            return;

        case seekmode_End:
            FROM_STRID(str)->setPosition(pos, std::ios_base::end);
            return;

        case seekmode_Current:
            FROM_STRID(str)->setPosition(pos, std::ios_base::cur);
            return;
    }
}

glui32 glk_stream_get_position(strid_t str) {
    glui32 pos = FROM_STRID(str)->position();

    SPDLOG_TRACE("glk_stream_get_pos({}) => {}", wrap::ptr(str), pos);

    return pos;
}

void glk_put_char(unsigned char ch) {
    SPDLOG_TRACE("glk_put_char({})", QString(ch));

    if(s_CurrentStream)
        s_CurrentStream->writeChar(ch);
}

void glk_put_char_stream(strid_t str, unsigned char ch) {
    SPDLOG_TRACE("glk_put_char_stream({}, {})", wrap::ptr(str), QString(ch));

    FROM_STRID(str)->writeChar(ch);
}

void glk_put_string(char* s) {
    SPDLOG_TRACE("glk_put_string({})", QString(s));

    if(s_CurrentStream)
        s_CurrentStream->writeString(s);
}

void glk_put_string_stream(strid_t str, char* s) {
    SPDLOG_TRACE("glk_put_string({}, {})", wrap::ptr(str), QString(s));

    FROM_STRID(str)->writeString(s);
}

void glk_put_buffer(char* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer({})", QString::fromLatin1(buf, len));

    if(s_CurrentStream)
        s_CurrentStream->writeBuffer(buf, len);
}

void glk_put_buffer_stream(strid_t str, char* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer_stream({}, {})", wrap::ptr(str), QString::fromLatin1(buf, len));

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

    SPDLOG_TRACE("glk_get_char_stream({}) => {}", wrap::ptr(str), ch);

    return ch;
}

glui32 glk_get_line_stream(strid_t str, char* buf, glui32 len) {
    glui32 count = FROM_STRID(str)->readLine(buf, len);

    SPDLOG_TRACE("glk_get_line_stream({}, {}, {}) => {}", wrap::ptr(str), (void*)buf, len, QString(buf));

    return count;
}

glui32 glk_get_buffer_stream(strid_t str, char* buf, glui32 len) {
    glui32 count = FROM_STRID(str)->readBuffer(buf, len);

    SPDLOG_TRACE("glk_get_buffer_stream({}, {}, {}) => {}", wrap::ptr(str), (void*)buf, len, count);

    return count;
}

void glk_put_char_uni(glui32 ch) {
    SPDLOG_TRACE("glk_put_char_uni({})", QString::fromUcs4(&ch, 1));

    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeChar(ch);
}

void glk_put_string_uni(glui32* s) {
    SPDLOG_TRACE("glk_put_string_uni({})", QString::fromUcs4(s));

    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeString(s);
}

void glk_put_buffer_uni(glui32* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer_uni({})", QString::fromUcs4(buf, len));

    if(s_CurrentStream)
        s_CurrentStream->writeUnicodeBuffer(buf, len);
}

void glk_put_char_stream_uni(strid_t str, glui32 ch) {
    SPDLOG_TRACE("glk_put_char_stream_uni({}, {})", wrap::ptr(str), QString::fromUcs4(&ch, 1));

    FROM_STRID(str)->writeUnicodeChar(ch);
}

void glk_put_string_stream_uni(strid_t str, glui32* s) {
    SPDLOG_TRACE("glk_put_string_stream_uni({}, {})", wrap::ptr(str), QString::fromUcs4(s));

    FROM_STRID(str)->writeUnicodeString(s);
}

void glk_put_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer_stream_uni({}, {})", wrap::ptr(str), QString::fromUcs4(buf, len));

    FROM_STRID(str)->writeUnicodeBuffer(buf, len);
}

glsi32 glk_get_char_stream_uni(strid_t str) {
    glsi32 ch = FROM_STRID(str)->readUnicodeChar();

    SPDLOG_TRACE("glk_get_char_stream_uni({}) => {}", wrap::ptr(str), ch);

    return ch;
}

glui32 glk_get_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    glui32 count = FROM_STRID(str)->readUnicodeBuffer(buf, len);

    SPDLOG_TRACE("glk_get_buffer_stream_uni({}, {}, {}) => {}", wrap::ptr(str), (void*)buf, len, count);

    return count;
}

glui32 glk_get_line_stream_uni(strid_t str, glui32* buf, glui32 len) {
    glui32 count = FROM_STRID(str)->readUnicodeLine(buf, len);

    SPDLOG_TRACE("glk_get_line_stream_uni({}, {}, {}) => {}", wrap::ptr(str), (void*)buf, len, QString::fromUcs4(buf, count));

    return count;
}

strid_t glk_stream_open_memory(char* buf, glui32 buflen, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_memory({}, {}, {}, {})", (void*)buf, buflen, wrap::filemode(fmode), rock);

    std::unique_ptr<std::streambuf> streambuf;
    if(buf) {
        if(fmode == filemode_Read)
            streambuf = std::make_unique<Glk::RegisteredMemBuf<true>>(buf, buflen, false);
        else
            streambuf = std::make_unique<Glk::RegisteredMemBuf<false>>(buf, buflen, false);

        if(fmode == filemode_WriteAppend)
            streambuf->pubseekoff(0, std::ios_base::end);
    } else {
        streambuf = std::make_unique<Glk::NullBuf>();
    }

    return TO_STRID(new Glk::Latin1Stream{nullptr, std::move(streambuf), Glk::Stream::Type::Memory, false, rock});
}

strid_t glk_stream_open_memory_uni(glui32* buf, glui32 buflen, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_memory_uni({}, {}, {}, {})", (void*)buf, buflen, wrap::filemode(fmode), rock);

    std::unique_ptr<std::streambuf> streambuf;
    if(buf) {
        if(fmode == filemode_Read)
            streambuf = std::make_unique<Glk::RegisteredMemBuf<true>>((char*)buf, sizeof(glui32)*buflen, true);
        else
            streambuf = std::make_unique<Glk::RegisteredMemBuf<false>>((char*)buf, sizeof(glui32)*buflen, true);

        if(fmode == filemode_WriteAppend)
            streambuf->pubseekoff(0, std::ios_base::end);
    } else {
        streambuf = std::make_unique<Glk::NullBuf>();
    }

    return TO_STRID(new Glk::UnicodeStream{nullptr, std::move(streambuf), Glk::Stream::Type::Memory, false, rock});
}

strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_file({}, {}, {})", wrap::ptr(fileref), wrap::filemode(fmode), rock);

    std::unique_ptr<std::filebuf> filebuf = std::make_unique<std::filebuf>();
    {
        auto path = FROM_FREFID(fileref)->path();

        std::filebuf* openbuf{};
        switch(fmode) {
            case filemode_Write:
                openbuf = filebuf->open(path, std::ios_base::out);
                break;

            case filemode_Read:
                openbuf = filebuf->open(path, std::ios_base::in);
                break;

            case filemode_ReadWrite:
                openbuf = filebuf->open(path, std::ios_base::in | std::ios_base::out);
                break;

            case filemode_WriteAppend:
                openbuf = filebuf->open(path, std::ios_base::out | std::ios_base::ate);
                break;
        }

        if(!openbuf) {
            spdlog::warn("Failed to open '{}' file stream for {}", wrap::filemode(fmode), wrap::ptr(fileref));
            return NULL;
        }
    }

    bool textMode = false;
    if((FROM_FREFID(fileref)->usage() & 0x100) == Glk::FileReference::TextMode)
        textMode = true;

    return TO_STRID(new Glk::Latin1Stream{nullptr, std::move(filebuf), Glk::Stream::Type::File, textMode, rock});
}

strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_file_uni({}, {}, {})", wrap::ptr(fileref), wrap::filemode(fmode), rock);

    std::unique_ptr<std::filebuf> filebuf = std::make_unique<std::filebuf>();
    {
        auto path = FROM_FREFID(fileref)->path();

        std::filebuf* openbuf{};
        switch(fmode) {
            case filemode_Write:
                openbuf = filebuf->open(path, std::ios_base::out);
                break;

            case filemode_Read:
                openbuf = filebuf->open(path, std::ios_base::in);
                break;

            case filemode_ReadWrite:
                openbuf = filebuf->open(path, std::ios_base::in | std::ios_base::out);
                break;

            case filemode_WriteAppend:
                openbuf = filebuf->open(path, std::ios_base::out | std::ios_base::ate);
                break;
        }

        if(!openbuf) {
            spdlog::warn("Failed to open '{}' file stream for {}", wrap::filemode(fmode), wrap::ptr(fileref));
            return NULL;
        }
    }

    bool textMode = false;
    if((FROM_FREFID(fileref)->usage() & 0x100) == Glk::FileReference::TextMode)
        textMode = true;

    return TO_STRID(new Glk::UnicodeStream{nullptr, std::move(filebuf), Glk::Stream::Type::File, textMode, rock});
}

strid_t glk_stream_open_resource(glui32 filenum, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_resource({}, {})", filenum, rock);

    Glk::Blorb::Chunk chunk{Glk::Blorb::loadResource(filenum)};
    if(!chunk.isValid())
        return NULL;

    bool textMode = false;
    if(chunk.type() == Glk::Blorb::Chunk::Type::TEXT)
        textMode = true;

    std::unique_ptr<std::streambuf> streambuf = std::make_unique<Glk::ChunkBuf>(std::move(chunk));

    return TO_STRID(new Glk::Latin1Stream{nullptr, std::move(streambuf), Glk::Stream::Type::Resource, textMode, rock});
}

strid_t glk_stream_open_resource_uni(glui32 filenum, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_resource_uni({}, {})", filenum, rock);

    Glk::Blorb::Chunk chunk{Glk::Blorb::loadResource(filenum)};
    if(!chunk.isValid())
        return NULL;

    bool textMode = false;
    if(chunk.type() == Glk::Blorb::Chunk::Type::TEXT)
        textMode = true;

    std::unique_ptr<std::streambuf> streambuf = std::make_unique<Glk::ChunkBuf>(std::move(chunk));

    return TO_STRID(new Glk::UnicodeStream{nullptr, std::move(streambuf), Glk::Stream::Type::Resource, textMode, rock});
}

void glk_set_hyperlink(glui32 linkval) {
    if(s_CurrentStream)
        s_CurrentStream->pushHyperlink(linkval);
}

void glk_set_hyperlink_stream(strid_t str, glui32 linkval) {
    FROM_STRID(str)->pushHyperlink(linkval);
}
