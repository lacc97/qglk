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

        struct splitmethod_wrap {
            glui32 splitmethod;
        };

        struct wintype_wrap {
            glui32 wintype;
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

    inline auto splitmethod(glui32 splitmethod) {
        return detail::splitmethod_wrap{splitmethod};
    }

    inline auto wintype(glui32 wintype) {
        return detail::wintype_wrap{wintype};
    }
}

namespace fmt {
    template <>
    struct formatter<wrap::detail::filemode_wrap> : formatter<std::string_view> {
        template <typename FormatContext>
        auto format(wrap::detail::filemode_wrap fmode, FormatContext& ctx) {
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
        auto format(wrap::detail::seekmode_wrap smode, FormatContext& ctx) {
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

    template <>
    struct formatter<wrap::detail::splitmethod_wrap> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(wrap::detail::splitmethod_wrap smethod, FormatContext& ctx) {
            using namespace std::string_view_literals;

            std::string_view dir;
            if((smethod.splitmethod & winmethod_Above) != 0)
                dir = (smethod.splitmethod & winmethod_Above) == winmethod_Above ? "Above"sv : "Below"sv;
            else
                dir = (smethod.splitmethod & winmethod_Right) == winmethod_Right ? "Right"sv : "Left"sv;
            std::string_view size = (smethod.splitmethod & winmethod_Proportional) != 0 ? "Proportional"sv : "Fixed"sv;
            std::string_view border = (smethod.splitmethod & winmethod_NoBorder) == 0 ? "Border"sv : "NoBorder"sv;

            return fmt::format_to(ctx.out(), "{0} | {1} | {2}", dir, size, border);
        }
    };

    template <>
    struct formatter<wrap::detail::wintype_wrap> : formatter<std::string_view> {
        template <typename FormatContext>
        auto format(wrap::detail::wintype_wrap wtype, FormatContext& ctx) {
            using namespace std::string_view_literals;

            auto s = "<unknown wintype>"sv;
            switch(wtype.wintype) {
                case wintype_Blank:
                    s = "Blank"sv;
                    break;
                case wintype_Graphics:
                    s = "Graphics"sv;
                    break;
                case wintype_Pair:
                    s = "Pair"sv;
                    break;
                case wintype_TextBuffer:
                    s = "TextBuffer"sv;
                    break;
                case wintype_TextGrid:
                    s = "TextGrid"sv;
                    break;
            }

            return formatter<std::string_view>::format(s, ctx);
        }
    };
}

#endif //QGLK_FORMAT_HPP
