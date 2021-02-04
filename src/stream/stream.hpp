#ifndef STREAM_STREAM_HPP
#define STREAM_STREAM_HPP

#include <cstring>

#include <memory>
#include <span>
#include <streambuf>

#include <bit_cast.hpp>

#include <glk.hpp>

#include "window/style.hpp"

#include "object.hpp"

namespace qglk {
    using stream = glk_stream_struct;
    using stream_driver = std::streambuf;
}    // namespace qglk

struct glk_stream_struct final : public qglk::object {
  public:
    enum type { eMemory = 1, eFile = 2, eResource = 3, eWindow = 4 };

    explicit glk_stream_struct(glui32 rock) : qglk::object{rock, qglk::object::eStream} {}

    void init(type type, bool unicode, bool text, std::unique_ptr<qglk::stream_driver> driver) noexcept;
    void destroy() noexcept;

    [[nodiscard]] glsi32 read_char() noexcept {
        char ch;
        return (read_buffer(&ch, 1) == 1) ? bit_cast<unsigned char>(ch) : -1;
    }
    [[nodiscard]] glui32 read_line(char* buf, glui32 len) noexcept;
    [[nodiscard]] glui32 read_buffer(char* buf, glui32 len) noexcept;

    [[nodiscard]] glsi32 read_unicode_char() noexcept {
        glui32 ch;
        return (read_unicode_buffer(&ch, 1) == 1) ? bit_cast<glsi32>(ch) : -1;
    }
    [[nodiscard]] glui32 read_unicode_line(glui32* buf, glui32 len) noexcept;
    [[nodiscard]] glui32 read_unicode_buffer(glui32* buf, glui32 len) noexcept;

    void push_hyperlink(glui32 linkval) noexcept;
    void push_style(Glk::Style::Type type) noexcept;

    void write_char(char ch) noexcept {
        write_buffer(&ch, 1);
    }
    void write_string(char* s) noexcept {
        write_buffer(s, std::strlen(s));
    }
    void write_buffer(char* buf, glui32 len) noexcept;

    void write_unicode_char(glui32 ch) noexcept {
        write_unicode_buffer(&ch, 1);
    }
    void write_unicode_string(glui32* s) noexcept {
        write_unicode_buffer(s, std::basic_string_view<glui32>(s).length());
    }
    void write_unicode_buffer(glui32* buf, glui32 len) noexcept;

    glui32 get_position() noexcept;
    void set_position(glui32 pos, glui32 seekmode) noexcept;

    [[nodiscard]] strid_t get_echo() const noexcept {
        return mp_echo;
    }
    void set_echo(strid_t echo) noexcept {
        mp_echo = echo;
    }

    [[nodiscard]] glui32 get_read_count() const noexcept {
        return m_read_count;
    }
    [[nodiscard]] glui32 get_write_count() const noexcept {
        return m_write_count;
    }

  private:
    static inline char from_unicode(glui32 ch) noexcept {
        return (ch >= 0x100u) ? '?' : bit_cast<char>(static_cast<unsigned char>(ch));
    }
    static inline glui32 to_unicode(char ch) noexcept {
        return bit_cast<unsigned char>(ch);
    }


    type m_type{};
    bool m_unicode : 1 {};
    bool m_text : 1 {};
    bool m_big_endian : 1 {};
    std::unique_ptr<qglk::stream_driver> mp_driver;

    glui32 m_read_count{}, m_write_count{};

    strid_t mp_echo{};
};

#endif
