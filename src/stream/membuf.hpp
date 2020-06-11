#ifndef QGLK_MEMBUF_HPP
#define QGLK_MEMBUF_HPP

#include <streambuf>

#include <buffer/buffer_span.hpp>
#include <buffer/buffer_view.hpp>

#include "qglk.hpp"

namespace Glk {
    template <bool ReadOnly>
    class MemBuf : public std::streambuf {
            using buffer_type = std::conditional_t<ReadOnly, buffer::byte_buffer_view, buffer::byte_buffer_span>;
        public:
            using char_type = std::conditional_t<ReadOnly, const char, char>;


            MemBuf(char_type* buffer_, glui32 length_);

        protected:
            int_type overflow(int_type ch) final;
            int_type underflow() final;

            pos_type seekoff(off_type type, std::ios_base::seekdir seekdir, std::ios_base::openmode openmode) final;
            pos_type seekpos(pos_type type, std::ios_base::openmode openmode) final;

            std::streamsize xsgetn(char* s, std::streamsize count) final;
            std::streamsize xsputn(const char* s, std::streamsize count) final;


            [[nodiscard]] inline const buffer_type& buffer() const {
                return m_Buffer;
            }

        private:
            buffer_type m_Buffer;

            std::streamsize m_Position{0};
    };

    template <bool ReadOnly>
    class RegisteredMemBuf final : public MemBuf<ReadOnly> {
        public:
            RegisteredMemBuf(typename MemBuf<ReadOnly>::char_type* buffer_, glui32 length_, bool unicode_);
            ~RegisteredMemBuf();

        private:
            bool m_Unicode;
    };
}

extern template class Glk::MemBuf<false>;
extern template class Glk::MemBuf<true>;

extern template class Glk::RegisteredMemBuf<false>;
extern template class Glk::RegisteredMemBuf<true>;


#endif //QGLK_MEMBUF_HPP
