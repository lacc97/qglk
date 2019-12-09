#ifndef SCHANNEL_HPP
#define SCHANNEL_HPP

#include <QBuffer>
#include <QMediaPlayer>
#include <QSet>

#include <fmt/format.h>

#include "glk.hpp"

#include "blorb/chunk.hpp"

namespace Glk {
    class SoundRepeater : public QObject {
            Q_OBJECT
        public:
            SoundRepeater(QMediaPlayer& mp);

            inline void setRepeats(glui32 nr) {
                m_NumRepeat = nr;
            }

        public slots:
            void mediaPlayerStateChanged(QMediaPlayer::State state);

        private:
            QMediaPlayer& mr_Player;
            glui32 m_NumRepeat;
    };

    class SoundChannel : public Object {
        public:
            static const glui32 FullVolume = 0x10000;

            SoundChannel(glui32 volume_ = FullVolume, glui32 rock_ = 0);
            ~SoundChannel();

            Glk::Object::Type objectType() const override {
                return Object::Type::SoundChannel;
            }

            bool play(glui32 snd, glui32 repeats, bool notify);
            void pause();
            void unpause();
            void stop();

            void setVolume(glui32 volume, glui32 duration = 0, bool notify = false);

        private:
            QMediaPlayer m_Player;
            SoundRepeater m_Repeater;
            Glk::Blorb::Chunk m_Chunk;
            QBuffer m_Buffer;

            bool m_Paused;
    };
}

inline const schanid_t TO_SCHANID(Glk::SoundChannel* sch) {
    return reinterpret_cast<schanid_t>(sch);
}
inline Glk::SoundChannel* const FROM_SCHANID(schanid_t sch) {
    return reinterpret_cast<Glk::SoundChannel*>(sch);
}

namespace fmt {
    template <>
    struct formatter<Glk::SoundChannel> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const Glk::SoundChannel& w, FormatContext &ctx) {
            return format_to(ctx.out(), "({})", (void*)&w);
        }
    };

    template <>
    struct formatter<std::remove_pointer_t<schanid_t>> : formatter<Glk::SoundChannel> {
        template <typename FormatContext>
        inline auto format(std::remove_pointer_t<schanid_t>& w, FormatContext &ctx) {
            return formatter<Glk::SoundChannel>::format(*FROM_SCHANID(&w), ctx);
        }
    };
}

#endif
