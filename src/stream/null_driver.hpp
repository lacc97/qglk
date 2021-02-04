#ifndef QGLK_NULL_DRIVER_HPP
#define QGLK_NULL_DRIVER_HPP

#include <streambuf>

namespace qglk::stream_drivers {
    class null : public std::streambuf {
      protected:
        auto overflow(int_type ch = traits_type::eof()) -> int_type override;

        auto seekoff(off_type off, std::ios_base::seekdir seekdir, std::ios_base::openmode openmode) -> pos_type override;
        auto seekpos(pos_type off, std::ios_base::openmode openmode) -> pos_type override;

        auto xsputn(const char_type* s, std::streamsize count) -> std::streamsize override;

      private:
        std::streamsize m_position{0};
    };
}    // namespace qglk::stream_drivers

#endif    //QGLK_NULL_DRIVER_HPP
