#ifndef QGLK_WINDOW_STREAM_DRIVER_HPP
#define QGLK_WINDOW_STREAM_DRIVER_HPP

#include <streambuf>
#include <type_traits>

#include "window.hpp"

namespace qglk::stream_drivers {
    class window : public std::streambuf {
      public:
        explicit window(Glk::Window* win) noexcept : mp_Window{win} {}

        void push_hyperlink(glui32 link) noexcept {
            get_window()->pushHyperlink(link);
        }
        void push_style(Glk::Style::Type type) noexcept {
            get_window()->pushStyle(type);
        }

        template <class WindowT = Glk::Window>
        requires std::is_base_of_v<Glk::Window, WindowT> [[nodiscard]] auto get_window() const -> WindowT* {
            return static_cast<WindowT*>(mp_Window);
        }

      protected:
        auto overflow(int_type ch) -> int_type final {
            char c = traits_type::to_char_type(ch);
            if(xsputn(&c, 1) == 1)
                return traits_type::not_eof(ch);
            else
                return traits_type::eof();
        }

        auto xsputn(const char_type* s, std::streamsize count) -> std::streamsize override {
            return 0;
        }

      private:
        Glk::Window* mp_Window;
    };
}    // namespace qglk::stream_drivers

#endif    //QGLK_WINDOW_STREAM_DRIVER_HPP
