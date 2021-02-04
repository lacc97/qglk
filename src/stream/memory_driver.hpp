#ifndef QGLK_MEMORY_DRIVER_HPP
#define QGLK_MEMORY_DRIVER_HPP

#include <cstring>

#include <span>
#include <streambuf>

#include <glk.hpp>

#include "qglk.hpp"

namespace qglk::stream_drivers {
    template <bool ReadOnly>
    class memory : public std::streambuf {
      protected:
        using buffer_type = std::span<std::conditional_t<ReadOnly, const char, char>>;

      public:
        explicit memory(buffer_type buf) noexcept : m_Buffer{buf} {}

      protected:
        auto overflow(int_type ch) -> int_type final {
            if constexpr(!ReadOnly) {
                if(m_Position < std::streamsize(m_Buffer.size()))
                    m_Buffer[m_Position] = ch;
                m_Position += 1;
                return traits_type::not_eof(ch);
            } else {
                return std::streambuf::overflow(ch);
            }
        }
        auto underflow() -> int_type final {
            char_type ch;
            if(xsgetn(&ch, 1) == 1)
                return traits_type::to_int_type(ch);
            else
                return traits_type::eof();
        }
        auto seekoff(off_type off, std::ios_base::seekdir seekdir, std::ios_base::openmode openmode) -> pos_type final {
            switch(seekdir) {
                case std::ios_base::beg:
                    return seekpos(off, openmode);

                case std::ios_base::cur:
                    return seekpos(m_Position + off, openmode);

                case std::ios_base::end:
                    return seekpos(off_type(m_Buffer.size()) + off, openmode);
            }

            return m_Position;
        }
        auto seekpos(pos_type off, std::ios_base::openmode openmode) -> pos_type final {
            return (m_Position = off);
        }
        auto xsgetn(char* s, std::streamsize count) -> std::streamsize final {
            std::streamsize copy_count = 0;
            if(m_Position < std::streamsize(m_Buffer.size()))
                std::memcpy(s, m_Buffer.data() + m_Position, copy_count = std::min<std::streamsize>(count, m_Buffer.size() - m_Position));
            m_Position += copy_count;
            return copy_count;
        }
        auto xsputn(const char* s, std::streamsize count) -> std::streamsize final {
            if constexpr(!ReadOnly) {
                if(m_Position < std::streamsize(m_Buffer.size()))
                    std::memcpy(m_Buffer.data() + m_Position, s, std::min<std::streamsize>(count, m_Buffer.size() - m_Position));
                m_Position += count;
                return count;
            } else {
                return std::streambuf::xsputn(s, count);
            }
        }


        [[nodiscard]] inline auto get_buffer() const -> buffer_type {
            return m_Buffer;
        }

      private:
        buffer_type m_Buffer;

        std::streamsize m_Position{0};
    };

    template <bool ReadOnly>
    class registered_memory : public memory<ReadOnly> {
      public:
        explicit registered_memory(std::span<std::conditional_t<ReadOnly, const char, char>> buf) noexcept : memory<ReadOnly>{buf}, m_unicode{false} {
            QGlk::getMainWindow().dispatch().registerArray((void*) buf.data(), buf.size(), m_unicode);
        }
        explicit registered_memory(std::span<std::conditional_t<ReadOnly, const glui32, glui32>> buf) noexcept
            : memory<ReadOnly>{std::span(reinterpret_cast<std::conditional_t<ReadOnly, const char, char>*>(buf.data()), buf.size() * sizeof(glui32))},
              m_unicode{true} {
            QGlk::getMainWindow().dispatch().registerArray((void*) buf.data(), buf.size(), m_unicode);
        }
        ~registered_memory() {
            QGlk::getMainWindow().dispatch().unregisterArray((void*) this->get_buffer().data(),
                                                             m_unicode ? this->get_buffer().size() / sizeof(glui32) : this->get_buffer().size(), m_unicode);
        }

      private:
        bool m_unicode;
    };
}    // namespace qglk::stream_drivers

#endif    //QGLK_MEMORY_DRIVER_HPP
