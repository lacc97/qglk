#ifndef SCHANNEL_HPP
#define SCHANNEL_HPP

#include <QBuffer>
#include <QMediaPlayer>

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

#define TO_SCHANID(sch) (reinterpret_cast<schanid_t>(sch))
#define FROM_SCHANID(sch) (reinterpret_cast<Glk::SoundChannel*>(sch))

#endif
