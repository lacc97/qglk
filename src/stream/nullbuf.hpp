#ifndef QGLK_NULLBUF_HPP
#define QGLK_NULLBUF_HPP

#include <array>
#include <streambuf>

#include "qglk.hpp"

namespace Glk {
    class NullBuf final : public std::streambuf {
        protected:
            int_type overflow(int_type ch = traits_type::eof()) override;

            pos_type seekoff(off_type off, std::ios_base::seekdir seekdir, std::ios_base::openmode openmode) override;
            pos_type seekpos(pos_type off, std::ios_base::openmode openmode) override;

            std::streamsize xsputn(const char_type* s, std::streamsize count) override;

        private:
            std::streamsize m_Position{0};
    };
}

#endif //QGLK_NULLBUF_HPP
