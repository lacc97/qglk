#ifndef QGLK_FORMAT_HPP
#define QGLK_FORMAT_HPP

#include <fmt/format.h>

extern "C" {
#include "glk.h"
}

namespace wrap {
    namespace detail {
        template<class T>
        struct ptr_wrap {
            T* ptr;
        };

        struct seekmode_wrap {
            glui32 seekmode;
        };
    }

    template <class T>
    inline auto ptr(T* ptr) {
        return detail::ptr_wrap<T>{ptr};
    }

    inline auto seekmode(glui32 seekmode) {
        return detail::seekmode_wrap{seekmode};
    }
}

namespace fmt {
    template <class T>
    struct formatter<wrap::detail::ptr_wrap<T>> : formatter<T> {
        template <typename FormatContext>
        inline auto format(wrap::detail::ptr_wrap<T> p, FormatContext& ctx) {
            if(!p.ptr)
                return format_to(ctx.out(), "<null>");
            else
                return formatter<T>::format(*p.ptr, ctx);
        }
    };

    template <>
    struct formatter<wrap::detail::seekmode_wrap> : formatter<std::string_view> {
        template <typename FormatContext>
        inline auto format(wrap::detail::seekmode_wrap smode, FormatContext& ctx) {
            using namespace std::string_view_literals;

            auto s = "<unknown seekmode>"sv;
            switch(smode.seekmode) {
                case seekmode_Start:
                    s = "seek::beg"sv;
                    break;

                case seekmode_Current:
                    s = "seek::cur"sv;
                    break;

                case seekmode_End:
                    s = "seek::end"sv;
                    break;
            }

            return formatter<std::string_view>::format(s, ctx);
        }
    };
}

#endif //QGLK_FORMAT_HPP
