#ifndef LATIN_1_STREAM_HPP
#define LATIN_1_STREAM_HPP

#include "stream.hpp"

namespace Glk {
    class Latin1Stream final : public Stream {
            Q_OBJECT
        public:
            Latin1Stream(QObject* parent_, std::unique_ptr<std::streambuf> buf_, Type type_, bool text_, glui32 rock_);
            ~Latin1Stream();
            
            glui32 position() const final;
            void setPosition(glsi32 off, std::ios_base::seekdir dir) final;

            void writeBuffer(buffer::byte_buffer_view buf) override;

            void writeUnicodeBuffer(buffer::buffer_view<glui32> buf) override;

            glui32 readBuffer(buffer::byte_buffer_span buf) override;
            glui32 readLine(buffer::byte_buffer_span buf) override;

            glui32 readUnicodeBuffer(buffer::buffer_span<glui32> buf) override;
            glui32 readUnicodeLine(buffer::buffer_span<glui32> buf) override;
    };
}

#endif
