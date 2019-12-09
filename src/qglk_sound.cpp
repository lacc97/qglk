#include <cassert>

#include "glk.hpp"

#include "qglk.hpp"

#include "blorb/chunk.hpp"
#include "log/log.hpp"
#include "sound/schannel.hpp"
#include "thread/taskrequest.hpp"

schanid_t glk_schannel_create(glui32 rock) {
    return glk_schannel_create_ext(rock, Glk::SoundChannel::FullVolume);
}

schanid_t glk_schannel_create_ext(glui32 rock, glui32 volume) {
    Glk::SoundChannel* sch;

    Glk::sendTaskToEventThread([&] {sch = new Glk::SoundChannel(volume, rock);});

    return TO_SCHANID(sch);
}

void glk_schannel_destroy(schanid_t chan) {
    Glk::sendTaskToEventThread([&] {delete FROM_SCHANID(chan);});
}

glui32 glk_schannel_play(schanid_t chan, glui32 snd) {
    return glk_schannel_play_ext(chan, snd, 1, 0);
}

glui32 glk_schannel_play_ext(schanid_t chan, glui32 snd, glui32 repeats, glui32 notify) {
    glui32 res;

    Glk::sendTaskToEventThread([&] {res = ((FROM_SCHANID(chan)->play(snd, repeats, notify)) ? 1 : 0);});
//     res = ((FROM_SCHANID(chan)->play(snd, repeats, notify)) ? 1 : 0);

    return res;
}

glui32 glk_schannel_play_multi(schanid_t* chanarray, glui32 chancount, glui32* sndarray, glui32 soundcount, glui32 notify) {
    assert(chancount == soundcount);

    glui32 count = 0;

    for(glui32 ii = 0; ii < chancount; ii++)
        count += glk_schannel_play_ext(chanarray[ii], sndarray[ii], 1, notify);

    return count;
}

void glk_schannel_stop(schanid_t chan) {
    FROM_SCHANID(chan)->stop();
}

void glk_schannel_pause(schanid_t chan) {
    FROM_SCHANID(chan)->pause();
}

void glk_schannel_unpause(schanid_t chan) {
    FROM_SCHANID(chan)->unpause();
}

void glk_schannel_set_volume(schanid_t chan, glui32 vol) {
    glk_schannel_set_volume_ext(chan, vol, 0, 0);
}

void glk_schannel_set_volume_ext(schanid_t chan, glui32 vol, glui32 duration, glui32 notify) {
    FROM_SCHANID(chan)->setVolume(vol, duration, notify);
}

void glk_sound_load_hint(glui32 snd, glui32 flag) {
    switch(flag) {
        case 0:
            if(Glk::Blorb::isResourceLoaded(snd, Glk::Blorb::ResourceUsage::Sound)) {
                Glk::Blorb::Chunk c = Glk::Blorb::loadResource(snd, Glk::Blorb::ResourceUsage::Sound);
                Glk::Blorb::unloadChunk(c);
            }

            break;

        case 1:
            Glk::Blorb::loadResource(snd, Glk::Blorb::ResourceUsage::Sound);
            break;
    }
}

schanid_t glk_schannel_iterate(schanid_t schan, glui32* rockptr) {
    const auto& schanList = QGlk::getMainWindow().soundChannelList();

    if(schan == NULL) {
        if(schanList.empty()) {
            SPDLOG_TRACE("glk_schannel_iterate({}, {}) => {}", wrap::ptr(schan), wrap::ptr(rockptr), (void*)NULL);
            return NULL;
        }

        auto first = schanList.first();

        if(rockptr)
            *rockptr = first->rock();

        SPDLOG_TRACE("glk_schannel_iterate({}, {}) => {}", wrap::ptr(schan), wrap::ptr(rockptr), wrap::ptr(first));

        return TO_SCHANID(first);
    }

    auto it = schanList.cbegin();

    while(it != schanList.cend() && (*it++) != FROM_SCHANID(schan));

    if(it == schanList.cend()) {
        SPDLOG_TRACE("glk_schannel_iterate({}, {}) => {}", wrap::ptr(schan), wrap::ptr(rockptr), (void*)NULL);
        return NULL;
    }

    if(rockptr)
        *rockptr = (*it)->rock();

    SPDLOG_TRACE("glk_schannel_iterate({}, {}) => {}", wrap::ptr(schan), wrap::ptr(rockptr), wrap::ptr(*it));

    return TO_SCHANID(*it);
}

glui32 glk_schannel_get_rock(schanid_t chan) {
    return FROM_SCHANID(chan)->rock();
}
