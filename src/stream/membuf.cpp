#include "membuf.hpp"

#include <cassert>
#include <limits>

#include <buffer/buffer_view.hpp>

template<bool ReadOnly>
Glk::MemBuf<ReadOnly>::MemBuf(char_type* buffer_, glui32 length_)
        : m_Buffer{buffer_, length_} {}

template<bool ReadOnly>
typename Glk::MemBuf<ReadOnly>::int_type Glk::MemBuf<ReadOnly>::overflow(int_type ch) {
    if constexpr(!ReadOnly) {
        if(m_Position < (std::streamsize)m_Buffer.size())
            m_Buffer[m_Position] = ch;
        m_Position += 1;
        return traits_type::not_eof(ch);
    } else {
        return std::streambuf::overflow(ch);
    }
}

template<bool ReadOnly>
typename Glk::MemBuf<ReadOnly>::pos_type Glk::MemBuf<ReadOnly>::seekoff(off_type off, std::ios_base::seekdir seekdir,
                                                                        std::ios_base::openmode openmode) {
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

template<bool ReadOnly>
typename Glk::MemBuf<ReadOnly>::pos_type Glk::MemBuf<ReadOnly>::seekpos(pos_type off,
                                                                        std::ios_base::openmode openmode) {
    return (m_Position = off);
}

template<bool ReadOnly>
typename Glk::MemBuf<ReadOnly>::int_type Glk::MemBuf<ReadOnly>::underflow() {
    char ch;
    if(xsgetn(&ch, 1) == 1)
        return traits_type::to_int_type(ch);
    else
        return traits_type::eof();
}

template<bool ReadOnly>
std::streamsize Glk::MemBuf<ReadOnly>::xsgetn(char* s, std::streamsize n) {
    buffer::byte_buffer_span buf{s, static_cast<size_t>(n)};

    std::streamsize copycount = 0;
    if(m_Position < std::streamsize(m_Buffer.size()))
        copycount = buf.copy_from(m_Buffer.subview(m_Position));
    m_Position += copycount;

    return copycount;
}

template<bool ReadOnly>
std::streamsize Glk::MemBuf<ReadOnly>::xsputn(const char* s, std::streamsize count) {
    if constexpr(!ReadOnly) {
        buffer::buffer_view<char> buf{s, static_cast<size_t>(count)};

        if(m_Position < std::streamsize(m_Buffer.size()))
            buf.copy_to(m_Buffer.subspan(m_Position));
        m_Position += buf.size();

        return buf.size();
    } else {
        return std::streambuf::xsputn(s, count);
    }
}

template<bool ReadOnly>
Glk::RegisteredMemBuf<ReadOnly>::RegisteredMemBuf(typename MemBuf<ReadOnly>::char_type* buffer_, glui32 length_,
                                                  bool unicode_)
        : MemBuf<ReadOnly>(buffer_, length_),
          m_Unicode{unicode_} {
    QGlk::getMainWindow().dispatch().registerArray((void*) (this->buffer().data()),
                                                   m_Unicode ? this->buffer().size()/sizeof(glui32) : this->buffer().size(),
                                                   m_Unicode);
}

template<bool ReadOnly>
Glk::RegisteredMemBuf<ReadOnly>::~RegisteredMemBuf() {
    QGlk::getMainWindow().dispatch().unregisterArray((void*) (this->buffer().data()),
                                                     m_Unicode ? this->buffer().size()/sizeof(glui32) : this->buffer().size(),
                                                     m_Unicode);
}

template
class Glk::MemBuf<false>;

template
class Glk::MemBuf<true>;

template
class Glk::RegisteredMemBuf<false>;

template
class Glk::RegisteredMemBuf<true>;