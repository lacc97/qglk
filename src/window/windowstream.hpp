#ifndef WINDOWSTREAM_HPP
#define WINDOWSTREAM_HPP

#include "stream/unicodestream.hpp"

namespace Glk {
    class WindowStream : public UnicodeStream {
            Q_OBJECT
        public:
            WindowStream(QIODevice* device_);

            inline Glk::Stream* echoStream() const {
                return mp_EchoStream;
            }
            void setEchoStream(Glk::Stream* echo);

            void writeUnicodeBuffer(glui32* buf, glui32 len) override;
            void writeUnicodeChar(glui32 ch) override;
            void writeUnicodeString(glui32* str) override;

        public slots:
            void onEchoStreamClosed();
            
        private:
            Glk::Stream* mp_EchoStream;
    };
}

#endif
