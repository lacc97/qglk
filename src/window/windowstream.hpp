#ifndef WINDOWSTREAM_HPP
#define WINDOWSTREAM_HPP

#include "stream/unicodestream.hpp"

namespace Glk {
    class Window;

    class WindowDevice : public QIODevice {
        public:
            explicit WindowDevice(Window* win);

            ~WindowDevice() override = default;

            template<class WindowT = Window>
            [[nodiscard]] inline WindowT* window() const {
                static_assert(std::is_base_of_v<Window, WindowT>);
                assert(dynamic_cast<WindowT*>(mp_Window));

                return static_cast<WindowT*>(mp_Window);
            }

        protected:
            qint64 readData(char* data, qint64 maxlen) override;

            qint64 writeData(const char* data, qint64 len) override;

        private:
            Window* mp_Window;
    };

    class WindowStream : public UnicodeStream {
        Q_OBJECT
        public:
            explicit WindowStream(WindowDevice* dev);


            void writeUnicodeBuffer(buffer::buffer_view<glui32> buf) override;


            [[nodiscard]] inline Glk::Stream* echoStream() const {
                return mp_EchoStream;
            }

            void setEchoStream(Glk::Stream* echo);

            void pushStyle(Style::Type sty) override;

            void pushHyperlink(glui32 linkval) override;

            [[nodiscard]] inline WindowDevice* windowDevice() const {
                return static_cast<WindowDevice*>(device());
            }

        public slots:
            void onEchoStreamClosed();

        private:
            Glk::Stream* mp_EchoStream;
    };
}

#endif
