#ifndef UNICODE_STREAM_HPP
#define UNICODE_STREAM_HPP

#include "stream.hpp"

namespace Glk {
    class UnicodeStream : public Stream {
            Q_OBJECT
        public:
            UnicodeStream(QObject* parent_, std::unique_ptr<std::streambuf> buf_, Type type_, bool text_, glui32 rock_);
            virtual ~UnicodeStream();

            glui32 position() const override;
            void setPosition(glsi32 off, std::ios_base::seekdir dir) override;

            void writeBuffer(buffer::byte_buffer_view buf) override;

            void writeUnicodeBuffer(buffer::buffer_view<glui32> buf) override;

            glui32 readBuffer(buffer::byte_buffer_span buf) override;
            glui32 readLine(buffer::byte_buffer_span buf) override;

            glui32 readUnicodeBuffer(buffer::buffer_span<glui32> buf) override;
            glui32 readUnicodeLine(buffer::buffer_span<glui32> buf) override;

        private:
            bool isStreamBigEndian() const;
    };
}

#endif
