#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <memory>

#include <QIODevice>
#include <QColor>

#include <fmt/format.h>

#include <glk.hpp>

#include "object.hpp"
#include "log/format.hpp"

#include "stream/stream.hpp"

#include "style.hpp"
#include "windowcontroller.hpp"

namespace qglk::stream_drivers {
  class window;
}

namespace Glk {
    class PairWindow;

    class Window : public Object {
        public:
            enum Type : glui32 {
                Blank = wintype_Blank,
                Graphics = wintype_Graphics,
                Pair = wintype_Pair,
                TextBuffer = wintype_TextBuffer,
                TextGrid = wintype_TextGrid,
            };


            ~Window() override;


            [[nodiscard]] Object::Type objectType() const final {
                return Object::Window;
            }


            virtual void clearWindow() = 0;

            virtual bool drawImage(glui32 img, glsi32 param1, glsi32 param2, QSize size);

            virtual void eraseRect(const QRect& rect);

            virtual void fillRect(const QColor& color, const QRect& rect);

            virtual void flowBreak();

            virtual void moveCursor(glui32 x, glui32 y);

            virtual void pushHyperlink(glui32 linkValue);

            virtual void pushStyle(Glk::Style::Type style);

            virtual void setBackgroundColor(const QColor& color);

            [[nodiscard]] inline Glk::Window::Type windowType() const {
                return m_Type;
            }


            template<class ControllerT = WindowController>
            [[nodiscard]] inline ControllerT* controller() const {
                static_assert(std::is_base_of_v<WindowController, ControllerT>);
                assert(dynamic_cast<ControllerT*>(mp_Controller));

                return static_cast<ControllerT*>(mp_Controller);
            }

            [[nodiscard]] inline PairWindow* parent() const {
                return mp_Parent;
            }

            inline void setParent(PairWindow* parent) {
                mp_Parent = parent;
            }

            [[nodiscard]] inline QSize size() const {
                return mp_Controller->glkSize();
            }

            [[nodiscard]] inline qglk::stream* stream() const {
                return mp_Stream.get();
            }

        protected:
            Window(Type type, WindowController* winController, std::unique_ptr<qglk::stream_drivers::window> streambuf, PairWindow* winParent, glui32 rock = 0);

        private:
            struct stream_deleter {
                inline void operator()(qglk::stream* ptr) {
                    if(ptr) {
                        ptr->destroy();
                        delete ptr;
                    }
                }
            };

            Glk::Window::Type m_Type;
            WindowController* mp_Controller;
            std::unique_ptr<qglk::stream, stream_deleter> mp_Stream;
            PairWindow* mp_Parent;
    };
}

inline winid_t TO_WINID(Glk::Window* win) {
    return reinterpret_cast<winid_t>(win);
}

inline Glk::Window* FROM_WINID(winid_t win) {
    return reinterpret_cast<Glk::Window*>(win);
}

namespace fmt {
    template <>
    struct formatter<Glk::Window> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const Glk::Window& w, FormatContext &ctx) {
            return format_to(ctx.out(), "{} Window ({})", wrap::wintype(w.windowType()), (void*)(&w));
        }
    };

    template <>
    struct formatter<std::remove_pointer_t<winid_t>> : formatter<Glk::Window> {
        template <typename FormatContext>
        inline auto format(std::remove_pointer_t<winid_t>& w, FormatContext &ctx) {
            return formatter<Glk::Window>::format(*FROM_WINID(&w), ctx);
        }
    };
}

#endif
