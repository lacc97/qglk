#include "nullbuf.hpp"

/* we pretend there is always more space available for putting */
Glk::NullBuf::int_type Glk::NullBuf::overflow(int_type ch) {
    m_Position += 1;
    return traits_type::not_eof(ch);
}

Glk::NullBuf::pos_type Glk::NullBuf::seekoff(off_type off, std::ios_base::seekdir seekdir, std::ios_base::openmode openmode) {
    switch(seekdir) {
        case std::ios_base::beg:
            return seekpos(off, openmode);

        case std::ios_base::cur:
        case std::ios_base::end:
            return seekpos(m_Position + off, openmode);
    }

    return m_Position;
}

Glk::NullBuf::pos_type Glk::NullBuf::seekpos(pos_type off, std::ios_base::openmode openmode) {
    return (m_Position = off);
}

/* we pretend there is always more space available for putting */
std::streamsize Glk::NullBuf::xsputn(const char_type* s, std::streamsize count) {
    m_Position += count;
    return count;
}