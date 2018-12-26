#ifndef STREAM_STREAM_HPP
#define STREAM_STREAM_HPP

#include <QIODevice>
#include <QSet>

#include "glk.hpp"

#include "window/style.hpp"

namespace Glk {
    class Stream : public QObject, public Object {
            Q_OBJECT
        public:
            enum class Type {
                Memory, File, Resource, Window
            };

            Stream(QObject* parent_, QIODevice* device_, Type type_, bool unicode_ = false, glui32 rock_ = 0);
            virtual ~Stream();

            Glk::Object::Type objectType() const override;

            virtual bool open(QIODevice::OpenMode om);
            inline bool close() {
                if(mp_Device->isOpen()) {
                    mp_Device->close();

                    if(!mp_Device->isOpen())
                        emit closed();
                }

                return !mp_Device->isOpen();
            }

            inline Type type() const {
                return m_Type;
            }

            virtual void pushStyle(Style::Type sty);
            
            virtual void pushHyperlink(glui32 linkval) {}

            bool isOpen() const;

            inline bool isUnicode() const {
                return m_Unicode;
            }

            virtual void setPosition(glui32 pos) = 0;
            virtual glui32 position() const = 0;

            inline glui32 size() const {
                return mp_Device->size();
            }
            inline glui32 readCount() const {
                return m_ReadChars;
            }
            inline glui32 writeCount() const {
                return m_WriteChars;
            }
            inline bool isInTextMode() const {
                return mp_Device->isTextModeEnabled();
            }

            // ASCII write methods
            virtual void writeBuffer(char* buf, glui32 len) = 0;
            virtual void writeString(char* str) = 0;
            virtual void writeChar(unsigned char ch) = 0;

            // Unicode write methods
            virtual void writeUnicodeBuffer(glui32* buf, glui32 len) = 0;
            virtual void writeUnicodeString(glui32* str) = 0;
            virtual void writeUnicodeChar(glui32 ch) = 0;

            // ASCII read methods
            virtual glui32 readBuffer(char* buf, glui32 len) = 0;
            virtual glui32 readLine(char* buf, glui32 len) = 0;
            virtual glsi32 readChar() = 0;

            // Unicode read methods
            virtual glui32 readUnicodeBuffer(glui32* buf, glui32 len) = 0;
            virtual glui32 readUnicodeLine(glui32* buf, glui32 len) = 0;
            virtual glsi32 readUnicodeChar() = 0;

        signals:
            void closed();

        protected:
            inline QIODevice* device() const {
                return mp_Device;
            }
            inline void updateReadCount(glui32 charread) {
                m_ReadChars += charread;
            }
            inline void updateWriteCount(glui32 charwrit) {
                m_WriteChars += charwrit;
            }

        private:
            QIODevice* mp_Device;
            bool m_Unicode;
            glui32 m_ReadChars;
            glui32 m_WriteChars;

            Type m_Type;
    };
}

inline const strid_t TO_STRID(Glk::Stream* str) {
    return reinterpret_cast<strid_t>(str);
}
inline Glk::Stream* const FROM_STRID(strid_t str) {
    return reinterpret_cast<Glk::Stream*>(str);
}

#endif


