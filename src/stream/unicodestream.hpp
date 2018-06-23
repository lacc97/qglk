#ifndef UNICODE_STREAM_HPP
#define UNICODE_STREAM_HPP

#include "stream.hpp"

namespace Glk {
    class UnicodeStream : public Stream {
        public:
            UnicodeStream(QObject* parent_, QIODevice* device_, Type type_, void* userptr_ = NULL, glui32 rock_ = 0);
            virtual ~UnicodeStream();

            bool isUnicode() const override {
                return true;
            }

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
