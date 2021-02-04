#include "stream.hpp"

#include <QtEndian>

#include <buffer/small_buffer.hpp>

#include <spdlog/spdlog.h>

#include "qglk.hpp"

#include "window/window_stream_driver.hpp"

namespace {
    template <typename T>
    concept stream_char = std::is_same_v<T, char> || std::is_same_v<T, glui32>;

    template <stream_char T, bool IsBufBigEndian = false>
    inline auto find_line_length(std::span<T> buf) noexcept {
        constexpr T k_new_line = []() constexpr->T {
            if constexpr(std::is_same_v<T, char> || !IsBufBigEndian)
                return '\n';
            else
                return qToBigEndian<T>('\n');
        }
        ();

        auto nl_pos = std::find(buf.begin(), buf.end(), k_new_line);
        return std::distance(buf.begin(), nl_pos) + (nl_pos != buf.end() ? 0 : 1);    // we must count the newline if we found one
    }
}    // namespace

void glk_stream_struct::destroy() noexcept {
    QGlk::getMainWindow().streamList().remove(this);

    destroy_base();
}

glui32 glk_stream_struct::read_buffer(char* buf, glui32 len) noexcept {
    glui32 read_count = 0;
    if(m_unicode) {
        buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ubuf(len);
        read_count = mp_driver->sgetn(reinterpret_cast<char*>(ubuf.data()), ubuf.size() * sizeof(glui32)) / sizeof(glui32);
        if(m_big_endian)
            std::transform(ubuf.begin(), ubuf.begin() + read_count, buf, [](glui32 ch) -> char { return from_unicode(qFromBigEndian<glui32>(ch)); });
        else
            std::transform(ubuf.begin(), ubuf.begin() + read_count, buf, from_unicode);
    } else {
        read_count = mp_driver->sgetn(buf, len);
    }

    m_read_count += read_count;

    return read_count;
}

glui32 glk_stream_struct::read_line(char* buf, glui32 len) noexcept {
    auto old_pos = mp_driver->pubseekoff(0, std::ios::cur);

    glui32 read_count = 0;
    if(m_unicode) {
        buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ubuf(len - 1);
        glui32 peek_count = mp_driver->sgetn(reinterpret_cast<char*>(ubuf.data()), ubuf.size() * sizeof(glui32)) / sizeof(glui32);
        if(m_big_endian) {
            read_count = find_line_length<glui32, true>(std::span(ubuf.data(), peek_count));
            std::transform(ubuf.begin(), ubuf.begin() + read_count, buf, [](glui32 ch) -> char { return from_unicode(qFromBigEndian<glui32>(ch)); });
        } else {
            read_count = find_line_length<glui32, false>(std::span(ubuf.data(), peek_count));
            std::transform(ubuf.begin(), ubuf.begin() + read_count, buf, from_unicode);
        }
    } else {
        glui32 peek_count = mp_driver->sgetn(buf, len - 1);
        read_count = find_line_length<char>(std::span(buf, peek_count));
    }
    buf[read_count] = 0;

    m_read_count += read_count;
    mp_driver->pubseekpos(old_pos + std::streamoff(m_unicode ? read_count * sizeof(glui32) : read_count));

    return read_count;
}

glui32 glk_stream_struct::get_position() noexcept {
    return mp_driver->pubseekoff(0, std::ios_base::cur);
}

glui32 glk_stream_struct::read_unicode_buffer(glui32* buf, glui32 len) noexcept {
    glui32 read_count = 0;
    if(m_unicode) {
        read_count = mp_driver->sgetn(reinterpret_cast<char*>(buf), len * sizeof(glui32)) / sizeof(glui32);
        if(m_big_endian)
            std::transform(buf, buf + read_count, buf, [](glui32 ch) -> glui32 { return qFromBigEndian<glui32>(ch); });
    } else {
        buffer::small_buffer<char, BUFSIZ> abuf(len);
        read_count = mp_driver->sgetn(abuf.data(), abuf.size());
        std::transform(abuf.begin(), abuf.begin() + read_count, buf, to_unicode);
    }

    m_read_count += read_count;

    return read_count;
}

glui32 glk_stream_struct::read_unicode_line(glui32* buf, glui32 len) noexcept {
    auto old_pos = mp_driver->pubseekoff(0, std::ios::cur);

    glui32 read_count = 0;
    if(m_unicode) {
        glui32 peek_count = mp_driver->sgetn(reinterpret_cast<char*>(buf), (len - 1) * sizeof(glui32)) / sizeof(glui32);
        if(m_big_endian) {
            read_count = find_line_length<glui32, true>(std::span(buf, peek_count));
            std::transform(buf, buf + read_count, buf, [](glui32 ch) -> char { return from_unicode(qFromBigEndian<glui32>(ch)); });
        } else {
            read_count = find_line_length<glui32, false>(std::span(buf, peek_count));
            std::transform(buf, buf + read_count, buf, from_unicode);
        }
    } else {
        buffer::small_buffer<char, BUFSIZ> abuf(len - 1);
        glui32 peek_count = mp_driver->sgetn(abuf.data(), len - 1);
        read_count = find_line_length<char>(std::span(abuf.data(), peek_count));
        std::transform(abuf.begin(), abuf.begin() + read_count, buf, to_unicode);
    }
    buf[read_count] = 0;

    m_read_count += read_count;
    mp_driver->pubseekpos(old_pos + std::streamoff(m_unicode ? read_count * sizeof(glui32) : read_count));

    return read_count;
}

void glk_stream_struct::init(glk_stream_struct::type type, bool unicode, bool text, std::unique_ptr<qglk::stream_driver> driver) noexcept {
    init_base();

    QGlk::getMainWindow().streamList().push_back(this);

    m_type = type;
    m_unicode = unicode;
    m_text = text;
    m_big_endian = (!m_text && (m_type == eFile || m_type == eResource));
    mp_driver = std::move(driver);
}

void glk_stream_struct::push_hyperlink(glui32 linkval) noexcept {
    if(auto win_driver = dynamic_cast<qglk::stream_drivers::window*>(mp_driver.get()))
        win_driver->push_hyperlink(linkval);
    else
        spdlog::warn("Stream does not accept hyperlinks");
}

void glk_stream_struct::push_style(Glk::Style::Type type) noexcept {
    if(auto win_driver = dynamic_cast<qglk::stream_drivers::window*>(mp_driver.get()))
        win_driver->push_style(type);
    else
        spdlog::warn("Stream does not accept styles");
}

void glk_stream_struct::write_buffer(char* buf, glui32 len) noexcept {
    if(m_unicode) {
        buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ubuf(len);
        if(m_big_endian)
            std::transform(buf, buf + len, ubuf.begin(), [](char ch) -> glui32 { return qToBigEndian<glui32>(to_unicode(ch)); });
        else
            std::transform(buf, buf + len, ubuf.begin(), to_unicode);
        m_write_count += mp_driver->sputn(reinterpret_cast<char*>(ubuf.data()), ubuf.size() * sizeof(glui32)) / sizeof(glui32);
    } else {
        m_write_count += mp_driver->sputn(buf, len);
    }

    if(mp_echo)
        mp_echo->write_buffer(buf, len);
}

void glk_stream_struct::write_unicode_buffer(glui32* buf, glui32 len) noexcept {
    if(m_unicode) {
        buffer::small_buffer<glui32, BUFSIZ / sizeof(glui32)> ubuf;
        if(m_big_endian) {
            ubuf.resize(len);
            std::transform(buf, buf + len, ubuf.begin(), [](glui32 ch) -> glui32 { return qToBigEndian<glui32>(ch); });
            buf = ubuf.data();
        }
        m_write_count += mp_driver->sputn(reinterpret_cast<char*>(buf), len * sizeof(glui32)) / sizeof(glui32);
    } else {
        buffer::small_buffer<char, BUFSIZ> abuf(len);
        std::transform(buf, buf + len, abuf.begin(), from_unicode);
        m_write_count += mp_driver->sputn(abuf.data(), len);
    }

    if(mp_echo)
        mp_echo->write_unicode_buffer(buf, len);
}

void glk_stream_struct::set_position(glui32 pos, glui32 seekmode) noexcept {
    switch(seekmode) {
        case seekmode_Start:
            mp_driver->pubseekoff(pos, std::ios_base::beg);
            return;

        case seekmode_End:
            mp_driver->pubseekoff(pos, std::ios_base::end);
            return;

        case seekmode_Current:
            mp_driver->pubseekoff(pos, std::ios_base::cur);
            return;
    }
}
