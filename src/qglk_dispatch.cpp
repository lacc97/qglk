#include "glk.hpp"

#include <map>

#include <spdlog/spdlog.h>

#include "qglk.hpp"
#include "dispatch.hpp"

gidispatch_rock_t gidispatch_get_objrock(void* obj, glui32 objclass) {
    Glk::Object* glkobj;

    switch(static_cast<Glk::Object::Type>(objclass)) {
        case Glk::Object::Type::Window:
            glkobj = FROM_WINID(reinterpret_cast<winid_t>(obj));
            break;

        case Glk::Object::Type::Stream:
            return static_cast<strid_t>(obj)->get_dispatch_rock();

        case Glk::Object::Type::FileReference:
            glkobj = FROM_FREFID(reinterpret_cast<frefid_t>(obj));
            break;

        case Glk::Object::Type::SoundChannel:
            glkobj = FROM_SCHANID(reinterpret_cast<schanid_t>(obj));
            break;

        default:
            qCritical() << "Object" << (obj) << "has an invalid object type" << objclass;
            glk_exit();
    }

    return glkobj->dispatchRock();
}

void gidispatch_set_object_registry(gidispatch_rock_t (* reg)(void* obj, glui32 objclass),
                                    void (* unreg)(void* obj, glui32 objclass, gidispatch_rock_t objrock)) {
    auto& dispatch = QGlk::getMainWindow().dispatch();
    assert(!dispatch.mf_RegisterObject && !dispatch.mf_UnregisterObject);

    dispatch.mf_RegisterObject = reg;
    dispatch.mf_UnregisterObject = unreg;
    if(reg) {
        for(frefid_t fref = glk_fileref_iterate(nullptr, nullptr); fref; fref = glk_fileref_iterate(fref, nullptr))
            dispatch.registerObject(FROM_FREFID(fref));

        for(schanid_t schan = glk_schannel_iterate(nullptr, nullptr); schan; schan = glk_schannel_iterate(schan, nullptr))
            dispatch.registerObject(FROM_SCHANID(schan));

        for(strid_t str = glk_stream_iterate(nullptr, nullptr); str; str = glk_stream_iterate(str, nullptr))
            dispatch.registerObject(str);

        for(winid_t win = glk_window_iterate(nullptr, nullptr); win; win = glk_window_iterate(win, nullptr))
            dispatch.registerObject(FROM_WINID(win));
    }
}

void gidispatch_set_retained_registry(gidispatch_rock_t (* reg)(void* array, glui32 len, char* typecode),
                                      void (* unreg)(void* array, glui32 len, char* typecode,
                                                     gidispatch_rock_t objrock)) {
    auto& dispatch = QGlk::getMainWindow().dispatch();
    assert(!dispatch.mf_RegisterArray && !dispatch.mf_UnregisterArray);

    dispatch.mf_RegisterArray = reg;
    dispatch.mf_UnregisterArray = unreg;
}

