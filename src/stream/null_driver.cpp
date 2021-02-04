#include "null_driver.hpp"

/* we pretend there is always more space available for putting */
auto qglk::stream_drivers::null::overflow(int_type ch) -> int_type {
    m_position += 1;
    return traits_type::not_eof(ch);
}

auto qglk::stream_drivers::null::seekoff(off_type off, std::ios_base::seekdir seekdir, std::ios_base::openmode openmode) -> pos_type {
    switch(seekdir) {
        case std::ios_base::beg:
            return seekpos(off, openmode);

        case std::ios_base::cur:
        case std::ios_base::end:
            return seekpos(m_position + off, openmode);
    }

    return m_position;
}

auto qglk::stream_drivers::null::seekpos(pos_type off, std::ios_base::openmode openmode) -> pos_type {
    return (m_position = off);
}

/* we pretend there is always more space available for putting */
auto qglk::stream_drivers::null::xsputn(const char_type* s, std::streamsize count) -> std::streamsize {
    m_position += count;
    return count;
}