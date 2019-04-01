#include "schannel.hpp"

#include "qglk.hpp"

#include "log/log.hpp"

Glk::SoundRepeater::SoundRepeater(QMediaPlayer& mp) : QObject(), mr_Player(mp), m_NumRepeat(0) {
    connect(&mr_Player, &QMediaPlayer::stateChanged, this, &Glk::SoundRepeater::mediaPlayerStateChanged);
}

void Glk::SoundRepeater::mediaPlayerStateChanged(QMediaPlayer::State state) {
    if(state != QMediaPlayer::StoppedState || mr_Player.mediaStatus() != QMediaPlayer::EndOfMedia)
        return;

    if(m_NumRepeat == 0)
        return;

    m_NumRepeat--;
    mr_Player.play();
}

Glk::SoundChannel::SoundChannel(glui32 volume_, glui32 rock_) : Object(rock_), m_Player(), m_Repeater(m_Player), m_Chunk(), m_Buffer(), m_Paused(false) {
    m_Player.setVolume(100 * volume_ / FullVolume);
    
    Glk::Dispatch::registerObject(this);
    QGlk::getMainWindow().soundChannelList().append(this);
}

Glk::SoundChannel::~SoundChannel() {
    if(!QGlk::getMainWindow().soundChannelList().removeOne(this))
        log_warn() << "Sound channel " << (this) << " not found in sound channel list while removing";
    else
        log_trace() << "Sound channel " << (this) << " removed from sound channel list";

    Glk::Dispatch::unregisterObject(this);
}

bool Glk::SoundChannel::play(glui32 snd, glui32 repeats, bool notify) { //TODO handle notify
    m_Player.setMedia(QMediaContent());
    m_Buffer.close();

    Glk::Blorb::Chunk newc = Glk::Blorb::loadResource(snd, Glk::Blorb::ResourceUsage::Sound);

    if(!newc.isValid())
        return false;

    Glk::Blorb::unloadChunk(m_Chunk);
    m_Chunk = std::move(newc);

    m_Buffer.setData(QByteArray::fromRawData(m_Chunk.data(), m_Chunk.length()));
    m_Buffer.open(QIODevice::ReadOnly);
    m_Buffer.seek(0);

    m_Player.setMedia(QMediaContent(), &m_Buffer);

    if(repeats == 0)
        return false;

    m_Repeater.setRepeats(repeats - 1);

    if(!m_Paused)
        m_Player.play();

    return true;
}

void Glk::SoundChannel::pause() {
    m_Player.pause();
    m_Paused = true;
}

void Glk::SoundChannel::unpause() {
    if(m_Paused)
        m_Player.play();

    m_Paused = false;
}

void Glk::SoundChannel::stop() {
    m_Player.stop();
}

void Glk::SoundChannel::setVolume(glui32 volume, glui32 duration, bool notify) { //TODO handle duration and notify
    m_Player.setVolume(100 * volume / FullVolume);
}

#include "moc_schannel.cpp"
