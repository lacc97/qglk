#ifndef WINDOWSTREAM_HPP
#define WINDOWSTREAM_HPP

#include "stream/unicodestream.hpp"

namespace Glk {
    class Window;

    class WindowBuf : public std::streambuf {
        public:
            explicit WindowBuf(Window* win);

            ~WindowBuf() override = default;

            template<class WindowT = Window>
            [[nodiscard]] inline WindowT* window() const {
                static_assert(std::is_base_of_v<Window, WindowT>);
                assert(dynamic_cast<WindowT*>(mp_Window));

                return static_cast<WindowT*>(mp_Window);
            }

        protected:
            int_type overflow(int_type ch) final;

            std::streamsize xsputn(const char_type* s, std::streamsize count) override;

        private:
            Window* mp_Window;
    };

    class WindowStream : public UnicodeStream {
        Q_OBJECT
        public:
            explicit WindowStream(std::unique_ptr<WindowBuf> winbuf);


            void writeUnicodeBuffer(buffer::buffer_view<glui32> buf) override;


            [[nodiscard]] inline Glk::Stream* echoStream() const {
                return mp_EchoStream;
            }

            void setEchoStream(Glk::Stream* echo);

            void pushStyle(Style::Type sty) override;

            void pushHyperlink(glui32 linkval) override;

            [[nodiscard]] inline WindowBuf* windowBuf() const {
                return static_cast<WindowBuf*>(streambuf());
            }

        public slots:
            void onEchoStreamClosed();

        private:
            Glk::Stream* mp_EchoStream;
    };
}

#endif
