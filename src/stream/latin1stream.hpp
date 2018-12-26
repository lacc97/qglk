#ifndef LATIN_1_STREAM_HPP
#define LATIN_1_STREAM_HPP

#include "stream.hpp"

namespace Glk {
    class Latin1Stream : public Stream {
            Q_OBJECT
        public:
            Latin1Stream(QObject* parent_, QIODevice* device_, Type type_, glui32 rock_ = 0);
            ~Latin1Stream();
            
            glui32 position() const override;
            void setPosition(glui32 pos) override;

            void writeBuffer(char* buf, glui32 len) override;
            void writeChar(unsigned char ch) override;
            void writeString(char* str) override;

            void writeUnicodeBuffer(glui32* buf, glui32 len) override;
            void writeUnicodeChar(glui32 ch) override;
            void writeUnicodeString(glui32* str) override;

            glui32 readBuffer(char* buf, glui32 len) override;
            glsi32 readChar() override;
            glui32 readLine(char* buf, glui32 len) override;

            glui32 readUnicodeBuffer(glui32* buf, glui32 len) override;
            glsi32 readUnicodeChar() override;
            glui32 readUnicodeLine(glui32* buf, glui32 len) override;
    };
}

#endif
