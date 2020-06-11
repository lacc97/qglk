#ifndef STREAM_STREAM_HPP
#define STREAM_STREAM_HPP

#include <memory>
#include <streambuf>

#include <QObject>

#include <buffer/buffer_span.hpp>
#include <buffer/buffer_view.hpp>

#include <fmt/format.h>

#include "glk.hpp"

#include "window/style.hpp"

namespace Glk {
    class Stream : public QObject, public Object {
            Q_OBJECT
        public:
            enum class Type {
                Memory, File, Resource, Window
            };

            Stream(QObject* parent_, std::unique_ptr<std::streambuf> buf_, Type type_, bool text_, bool unicode_, glui32 rock_);
            virtual ~Stream();

            Glk::Object::Type objectType() const override;

            inline Type type() const {
                return m_Type;
            }

            virtual void pushStyle(Style::Type sty);
            
            virtual void pushHyperlink(glui32 linkval) {}

            inline bool isUnicode() const {
                return m_Unicode;
            }

            virtual void setPosition(glsi32 off, std::ios_base::seekdir dir) = 0;
            virtual glui32 position() const = 0;

            inline glui32 readCount() const {
                return m_ReadChars;
            }
            inline glui32 writeCount() const {
                return m_WriteChars;
            }
            inline bool isInTextMode() const {
                return m_TextMode;
            }


            // ASCII write methods
            inline void writeBuffer(char* buf, glui32 len) {
                writeBuffer(buffer::byte_buffer_view{buf, len});
            }
            inline void writeString(char* str) {
                writeBuffer(buffer::byte_buffer_view{str, std::basic_string_view<char>{str}.length()});
            }
            inline void writeChar(unsigned char ch) {
                writeBuffer(buffer::byte_buffer_view{reinterpret_cast<char*>(&ch), 1});
            }
            virtual void writeBuffer(buffer::byte_buffer_view buf) = 0;

            // Unicode write methods
            inline void writeUnicodeBuffer(glui32* buf, glui32 len) {
                writeUnicodeBuffer(buffer::buffer_view<glui32>{buf, len});
            }
            inline void writeUnicodeString(glui32* str) {
                writeUnicodeBuffer(buffer::buffer_view<glui32>{str, std::basic_string_view<glui32>{str}.length()});
            }
            inline void writeUnicodeChar(glui32 ch) {
                writeUnicodeBuffer(buffer::buffer_view<glui32>{&ch, 1});
            }
            virtual void writeUnicodeBuffer(buffer::buffer_view<glui32> buf) = 0;

            // ASCII read methods
            inline glsi32 readChar() {
                char ch;
                return (readBuffer(&ch, 1) == 1) ? ch : -1;
            }
            inline glui32 readBuffer(char* buf, glui32 len) {
                return readBuffer({buf, len});
            }
            virtual glui32 readBuffer(buffer::byte_buffer_span buf) = 0;
            inline glui32 readLine(char* buf, glui32 len) {
                return readLine({buf, len});
            }
            virtual glui32 readLine(buffer::byte_buffer_span buf) = 0;

            // Unicode read methods
            inline glsi32 readUnicodeChar() {
                glui32 ch;
                return (readUnicodeBuffer(&ch, 1) == 1) ? ch : -1;
            }
            inline glui32 readUnicodeBuffer(glui32* buf, glui32 len) {
                return readUnicodeBuffer({buf, len});
            }
            virtual glui32 readUnicodeBuffer(buffer::buffer_span<glui32> buf) = 0;
            inline glui32 readUnicodeLine(glui32* buf, glui32 len) {
                return readUnicodeLine({buf, len});
            }
            virtual glui32 readUnicodeLine(buffer::buffer_span<glui32> buf) = 0;

        signals:
            void closed();

        protected:
            inline std::streambuf* streambuf() const {
                return mp_Streambuf.get();
            }
            inline void updateReadCount(glui32 charread) {
                m_ReadChars += charread;
            }
            inline void updateWriteCount(glui32 charwrit) {
                m_WriteChars += charwrit;
            }

        private:
            Type m_Type;
            bool m_TextMode;
            bool m_Unicode;

            std::unique_ptr<std::streambuf> mp_Streambuf;

            glui32 m_ReadChars{0};
            glui32 m_WriteChars{0};
    };
}

inline strid_t TO_STRID(Glk::Stream* str) {
    return reinterpret_cast<strid_t>(str);
}
inline Glk::Stream* FROM_STRID(strid_t str) {
    return reinterpret_cast<Glk::Stream*>(str);
}

namespace fmt {
    template <>
    struct formatter<Glk::Stream> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const Glk::Stream& w, FormatContext &ctx) {
            return format_to(ctx.out(), "({})", (void*)&w);
        }
    };

    template <>
    struct formatter<std::remove_pointer_t<strid_t>> : formatter<Glk::Stream> {
        template <typename FormatContext>
        inline auto format(std::remove_pointer_t<strid_t>& w, FormatContext &ctx) {
            return formatter<Glk::Stream>::format(*FROM_STRID(&w), ctx);
        }
    };
}

#endif


