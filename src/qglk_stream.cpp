#include "file/fileref.hpp"

#include <cstring>

#include <fstream>
#include <memory>

#include "qglk.hpp"

#include "log/log.hpp"
#include "stream/chunk_driver.hpp"
#include "stream/memory_driver.hpp"
#include "stream/null_driver.hpp"

strid_t sp_CurrentStream = NULL;
void glk_stream_set_current(strid_t str) {
    SPDLOG_TRACE("glk_stream_set_current({})", wrap::ptr(str));

    sp_CurrentStream = str;
}

strid_t glk_stream_get_current(void) {
    SPDLOG_TRACE("glk_stream_get_current() => {}", wrap::ptr(s_CurrentStream));

    return sp_CurrentStream;
}

void glk_stream_close(strid_t str, stream_result_t* result) {
    SPDLOG_TRACE("glk_stream_close({}, {})", wrap::ptr(str), (void*) (result));

    if(result) {
        result->readcount = str->get_read_count();
        result->writecount = str->get_write_count();
    }

    if(sp_CurrentStream == str)
        sp_CurrentStream = nullptr;

    str->destroy();

    delete str;
}

strid_t glk_stream_iterate(strid_t str, glui32* p_rock) {
    auto* next_str = QGlk::getMainWindow().streamList().next(str);
    if(next_str && p_rock)
        *p_rock = next_str->get_rock();

    SPDLOG_TRACE("glk_stream_iterate({}, {}) => {}", wrap::ptr(str), wrap::ptr(p_rock), wrap::ptr(next_str));
    return next_str;
}

glui32 glk_stream_get_rock(strid_t str) {
    return str->get_rock();
}

void glk_stream_set_position(strid_t str, glsi32 pos, glui32 seekmode) {
    SPDLOG_TRACE("glk_stream_set_position({}, {}, {})", wrap::ptr(str), pos, wrap::seekmode(seekmode));

    str->set_position(pos, seekmode);
}

glui32 glk_stream_get_position(strid_t str) {
    glui32 pos = str->get_position();

    SPDLOG_TRACE("glk_stream_get_pos({}) => {}", wrap::ptr(str), pos);

    return pos;
}

void glk_put_char(unsigned char ch) {
    SPDLOG_TRACE("glk_put_char({})", QString(ch));

    if(sp_CurrentStream)
        sp_CurrentStream->write_char(bit_cast<char>(ch));
}

void glk_put_char_stream(strid_t str, unsigned char ch) {
    SPDLOG_TRACE("glk_put_char_stream({}, {})", wrap::ptr(str), QString(ch));

    str->write_char(bit_cast<char>(ch));
}

void glk_put_string(char* s) {
    SPDLOG_TRACE("glk_put_string({})", QString(s));

    if(sp_CurrentStream)
        sp_CurrentStream->write_string(s);
}

void glk_put_string_stream(strid_t str, char* s) {
    SPDLOG_TRACE("glk_put_string({}, {})", wrap::ptr(str), QString(s));

    str->write_string(s);
}

void glk_put_buffer(char* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer({})", QString::fromLatin1(buf, len));

    if(sp_CurrentStream)
        sp_CurrentStream->write_buffer(buf, len);
}

void glk_put_buffer_stream(strid_t str, char* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer_stream({}, {})", wrap::ptr(str), QString::fromLatin1(buf, len));

    str->write_buffer(buf, len);
}

void glk_set_style(glui32 styl) {
    if(sp_CurrentStream)
        sp_CurrentStream->push_style(static_cast<Glk::Style::Type>(styl));
}

void glk_set_style_stream(strid_t str, glui32 styl) {
    str->push_style(static_cast<Glk::Style::Type>(styl));
}

glsi32 glk_get_char_stream(strid_t str) {
    glsi32 ch = str->read_char();

    SPDLOG_TRACE("glk_get_char_stream({}) => {}", wrap::ptr(str), ch);

    return ch;
}

glui32 glk_get_line_stream(strid_t str, char* buf, glui32 len) {
    glui32 count = str->read_line(buf, len);

    SPDLOG_TRACE("glk_get_line_stream({}, {}, {}) => {}", wrap::ptr(str), (void*) buf, len, QString(buf));

    return count;
}

glui32 glk_get_buffer_stream(strid_t str, char* buf, glui32 len) {
    glui32 count = str->read_buffer(buf, len);

    SPDLOG_TRACE("glk_get_buffer_stream({}, {}, {}) => {}", wrap::ptr(str), (void*) buf, len, count);

    return count;
}

void glk_put_char_uni(glui32 ch) {
    SPDLOG_TRACE("glk_put_char_uni({})", QString::fromUcs4(&ch, 1));

    if(sp_CurrentStream)
        sp_CurrentStream->write_unicode_char(ch);
}

void glk_put_char_stream_uni(strid_t str, glui32 ch) {
    SPDLOG_TRACE("glk_put_char_stream_uni({}, {})", wrap::ptr(str), QString::fromUcs4(&ch, 1));

    str->write_unicode_char(ch);
}

void glk_put_string_uni(glui32* s) {
    SPDLOG_TRACE("glk_put_string_uni({})", QString::fromUcs4(s));

    if(sp_CurrentStream)
        sp_CurrentStream->write_unicode_string(s);
}

void glk_put_string_stream_uni(strid_t str, glui32* s) {
    SPDLOG_TRACE("glk_put_string_stream_uni({}, {})", wrap::ptr(str), QString::fromUcs4(s));

    str->write_unicode_string(s);
}

void glk_put_buffer_uni(glui32* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer_uni({})", QString::fromUcs4(buf, len));

    if(sp_CurrentStream)
        sp_CurrentStream->write_unicode_buffer(buf, len);
}

void glk_put_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    SPDLOG_TRACE("glk_put_buffer_stream_uni({}, {})", wrap::ptr(str), QString::fromUcs4(buf, len));

    str->write_unicode_buffer(buf, len);
}

glsi32 glk_get_char_stream_uni(strid_t str) {
    glsi32 ch = str->read_unicode_char();

    SPDLOG_TRACE("glk_get_char_stream_uni({}) => {}", wrap::ptr(str), ch);

    return ch;
}

glui32 glk_get_buffer_stream_uni(strid_t str, glui32* buf, glui32 len) {
    glui32 count = str->read_unicode_buffer(buf, len);

    SPDLOG_TRACE("glk_get_buffer_stream_uni({}, {}, {}) => {}", wrap::ptr(str), (void*) buf, len, count);

    return count;
}

glui32 glk_get_line_stream_uni(strid_t str, glui32* buf, glui32 len) {
    glui32 count = str->read_unicode_line(buf, len);

    SPDLOG_TRACE("glk_get_line_stream_uni({}, {}, {}) => {}", wrap::ptr(str), (void*) buf, len, QString::fromUcs4(buf, count));

    return count;
}

strid_t glk_stream_open_memory(char* buf, glui32 buflen, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_memory({}, {}, {}, {})", (void*) buf, buflen, wrap::filemode(fmode), rock);

    auto stream = std::make_unique<qglk::stream>(rock);

    std::unique_ptr<std::streambuf> streambuf;
    if(buf) {
        if(fmode == filemode_Read)
            streambuf = std::make_unique<qglk::stream_drivers::registered_memory<true>>(std::span(buf, buflen));
        else
            streambuf = std::make_unique<qglk::stream_drivers::registered_memory<false>>(std::span(buf, buflen));

        if(fmode == filemode_WriteAppend)
            streambuf->pubseekoff(0, std::ios_base::end);
    } else {
        streambuf = std::make_unique<qglk::stream_drivers::null>();
    }

    stream->init(qglk::stream::eMemory, false, false, std::move(streambuf));

    return stream.release();
}

strid_t glk_stream_open_memory_uni(glui32* buf, glui32 buflen, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_memory_uni({}, {}, {}, {})", (void*) buf, buflen, wrap::filemode(fmode), rock);

    auto stream = std::make_unique<qglk::stream>(rock);

    std::unique_ptr<std::streambuf> streambuf;
    if(buf) {
        if(fmode == filemode_Read)
            streambuf = std::make_unique<qglk::stream_drivers::registered_memory<true>>(std::span(buf, buflen));
        else
            streambuf = std::make_unique<qglk::stream_drivers::registered_memory<false>>(std::span(buf, buflen));

        if(fmode == filemode_WriteAppend)
            streambuf->pubseekoff(0, std::ios_base::end);
    } else {
        streambuf = std::make_unique<qglk::stream_drivers::null>();
    }

    stream->init(qglk::stream::eMemory, true, false, std::move(streambuf));

    return stream.release();
}

strid_t glk_stream_open_file(frefid_t fileref, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_file({}, {}, {})", wrap::ptr(fileref), wrap::filemode(fmode), rock);

    auto stream = std::make_unique<qglk::stream>(rock);

    std::unique_ptr<std::filebuf> filebuf = std::make_unique<std::filebuf>();
    {
        const auto& path = fileref->get_path();

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
            return nullptr;
        }
    }

    bool textMode = false;
    if((fileref->get_usage() & qglk::file_reference::eTextMode) == qglk::file_reference::eTextMode)
        textMode = true;

    stream->init(qglk::stream::eFile, false, textMode, std::move(filebuf));

    return stream.release();
}

strid_t glk_stream_open_file_uni(frefid_t fileref, glui32 fmode, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_file_uni({}, {}, {})", wrap::ptr(fileref), wrap::filemode(fmode), rock);

    auto stream = std::make_unique<qglk::stream>(rock);

    std::unique_ptr<std::filebuf> filebuf = std::make_unique<std::filebuf>();
    {
        const auto& path = fileref->get_path();

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
            return nullptr;
        }
    }

    bool textMode = false;
    if((fileref->get_usage() & qglk::file_reference::eTextMode) == qglk::file_reference::eTextMode)
        textMode = true;

    stream->init(qglk::stream::eFile, true, textMode, std::move(filebuf));

    return stream.release();
}

strid_t glk_stream_open_resource(glui32 filenum, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_resource({}, {})", filenum, rock);

    auto stream = std::make_unique<qglk::stream>(rock);

    Glk::Blorb::Chunk chunk{Glk::Blorb::loadResource(filenum)};
    if(!chunk.isValid())
        return nullptr;

    bool textMode = false;
    if(chunk.type() == Glk::Blorb::Chunk::Type::TEXT)
        textMode = true;

    stream->init(qglk::stream::eResource, false, textMode, std::make_unique<qglk::stream_drivers::chunk>(std::move(chunk)));

    return stream.release();
}

strid_t glk_stream_open_resource_uni(glui32 filenum, glui32 rock) {
    SPDLOG_TRACE("glk_stream_open_resource_uni({}, {})", filenum, rock);

    auto stream = std::make_unique<qglk::stream>(rock);

    Glk::Blorb::Chunk chunk{Glk::Blorb::loadResource(filenum)};
    if(!chunk.isValid())
        return nullptr;

    bool textMode = false;
    if(chunk.type() == Glk::Blorb::Chunk::Type::TEXT)
        textMode = true;

    stream->init(qglk::stream::eResource, true, textMode, std::make_unique<qglk::stream_drivers::chunk>(std::move(chunk)));

    return stream.release();
}

void glk_set_hyperlink(glui32 linkval) {
    if(sp_CurrentStream)
        sp_CurrentStream->push_hyperlink(linkval);
}

void glk_set_hyperlink_stream(strid_t str, glui32 linkval) {
    str->push_hyperlink(linkval);
}
