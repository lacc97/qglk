#ifndef QGLK_FORMAT_HPP
#define QGLK_FORMAT_HPP

#include <fmt/format.h>

extern "C" {
#include "glk.h"
}

namespace wrap {
    namespace detail {
        struct filemode_wrap {
            glui32 filemode;
        };

        template<class T>
        struct ptr_wrap {
            T* ptr;
        };

        struct seekmode_wrap {
            glui32 seekmode;
        };
    }

    inline auto filemode(glui32 filemode) {
        return detail::filemode_wrap{filemode};
    }

    template <class T, std::enable_if_t<std::is_pointer_v<T> || std::is_null_pointer_v<T>, bool> = true>
    inline auto ptr(T ptr) {
        if constexpr(std::is_null_pointer_v<T>)
            return std::string_view{"<null>"};
        else
            return detail::ptr_wrap<std::remove_pointer_t<T>>{ptr};
    }

    inline auto seekmode(glui32 seekmode) {
        return detail::seekmode_wrap{seekmode};
    }
}

namespace fmt {
    template <>
    struct formatter<wrap::detail::filemode_wrap> : formatter<std::string_view> {
        template <typename FormatContext>
        inline auto format(wrap::detail::filemode_wrap fmode, FormatContext& ctx) {
            using namespace std::string_view_literals;

            auto s = "<unknown filemode>"sv;
            switch(fmode.filemode) {
                case filemode_Write:
                    s = "out"sv;
                    break;

                case filemode_Read:
                    s = "in"sv;
                    break;

                case filemode_ReadWrite:
                    s = "out | in"sv;
                    break;

                case filemode_WriteAppend:
                    s = "out | ate"sv;
                    break;
            }

            return formatter<std::string_view>::format(s, ctx);
        }
    };

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
