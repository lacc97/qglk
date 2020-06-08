#include "glk.hpp"

#include <map>

#include "file/fileref.hpp"
#include "sound/schannel.hpp"
#include "stream/stream.hpp"
#include "window/window.hpp"

#include "qglk.hpp"

void Glk::Dispatch::registerObject(Glk::Object* ptr) {
    if(mf_RegisterObject) {
        switch(ptr->objectType()) {
            case Glk::Object::Type::Window:
                assert(dynamic_cast<Glk::Window*>(ptr));
                ptr->m_DispatchRock = mf_RegisterObject(static_cast<Glk::Window*>(ptr), glui32(ptr->objectType()));
                return;

            case Glk::Object::Type::Stream:
                assert(dynamic_cast<Glk::Stream*>(ptr));
                ptr->m_DispatchRock = mf_RegisterObject(static_cast<Glk::Stream*>(ptr), glui32(ptr->objectType()));
                return;

            case Glk::Object::Type::FileReference:
                assert(dynamic_cast<Glk::FileReference*>(ptr));
                ptr->m_DispatchRock = mf_RegisterObject(static_cast<Glk::FileReference*>(ptr), glui32(ptr->objectType()));
                return;

            case Glk::Object::Type::SoundChannel:
                assert(dynamic_cast<Glk::SoundChannel*>(ptr));
                ptr->m_DispatchRock = mf_RegisterObject(static_cast<Glk::SoundChannel*>(ptr), glui32(ptr->objectType()));
                return;

            default:
                qCritical() << "Object" << (ptr) << "has an invalid object type" << glui32(ptr->objectType());
                glk_exit();
        }
    }
}

void Glk::Dispatch::unregisterObject(Glk::Object* ptr) {
    if(mf_UnregisterObject)
        switch(ptr->objectType()) {
            case Glk::Object::Type::Window:
                assert(dynamic_cast<Glk::Window*>(ptr));
                mf_UnregisterObject(static_cast<Glk::Window*>(ptr), glui32(ptr->objectType()), ptr->m_DispatchRock);
                return;

            case Glk::Object::Type::Stream:
                assert(dynamic_cast<Glk::Stream*>(ptr));
                mf_UnregisterObject(static_cast<Glk::Stream*>(ptr), glui32(ptr->objectType()), ptr->m_DispatchRock);
                return;

            case Glk::Object::Type::FileReference:
                assert(dynamic_cast<Glk::FileReference*>(ptr));
                mf_UnregisterObject(static_cast<Glk::FileReference*>(ptr), glui32(ptr->objectType()), ptr->m_DispatchRock);
                return;

            case Glk::Object::Type::SoundChannel:
                assert(dynamic_cast<Glk::SoundChannel*>(ptr));
                mf_UnregisterObject(static_cast<Glk::SoundChannel*>(ptr), glui32(ptr->objectType()), ptr->m_DispatchRock);
                return;

            default:
                qCritical() << "Object" << (ptr) << "has an invalid object type" << glui32(ptr->objectType());
                glk_exit();
        }
}

const char CHAR_CODE[] = "&+#!Cn";
const char UCS4_CODE[] = "&+#!Iu";

void Glk::Dispatch::registerArray(void* ptr, glui32 len, bool unicode) {
    if(mf_RegisterArray)
        m_ArrayRegistry[ptr] = mf_RegisterArray(ptr, len, (char*) (unicode ? UCS4_CODE : CHAR_CODE));
}

void Glk::Dispatch::unregisterArray(void* ptr, glui32 len, bool unicode) {
    if(m_ArrayRegistry.find(ptr) != m_ArrayRegistry.end()) {
        if(mf_UnregisterArray)
            mf_UnregisterArray(ptr, len, (char*) (unicode ? UCS4_CODE : CHAR_CODE), m_ArrayRegistry[ptr]);

        m_ArrayRegistry.erase(ptr);
    }
}


void gidispatch_set_object_registry(gidispatch_rock_t (* reg)(void* obj, glui32 objclass),
                                    void (* unreg)(void* obj, glui32 objclass, gidispatch_rock_t objrock)) {
    auto& dispatch = QGlk::getMainWindow().dispatch();
    assert(!dispatch.mf_RegisterObject && !dispatch.mf_UnregisterObject);

    dispatch.mf_RegisterObject = reg;
    dispatch.mf_UnregisterObject = unreg;
    if(reg) {
        for(frefid_t fref = glk_fileref_iterate(NULL, NULL); fref; fref = glk_fileref_iterate(fref, NULL))
            dispatch.registerObject(FROM_FREFID(fref));

        for(schanid_t schan = glk_schannel_iterate(NULL, NULL); schan; schan = glk_schannel_iterate(schan, NULL))
            dispatch.registerObject(FROM_SCHANID(schan));

        for(strid_t str = glk_stream_iterate(NULL, NULL); str; str = glk_stream_iterate(str, NULL))
            dispatch.registerObject(FROM_STRID(str));

        for(winid_t win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
            dispatch.registerObject(FROM_WINID(win));
    }
}

gidispatch_rock_t gidispatch_get_objrock(void* obj, glui32 objclass) {
    Glk::Object* glkobj;

    switch(static_cast<Glk::Object::Type>(objclass)) {
        case Glk::Object::Type::Window:
            glkobj = FROM_WINID(reinterpret_cast<winid_t>(obj));
            break;

        case Glk::Object::Type::Stream:
            glkobj = FROM_STRID(reinterpret_cast<strid_t>(obj));
            break;

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

void gidispatch_set_retained_registry(gidispatch_rock_t (* reg)(void* array, glui32 len, char* typecode),
                                      void (* unreg)(void* array, glui32 len, char* typecode,
                                                     gidispatch_rock_t objrock)) {
    auto& dispatch = QGlk::getMainWindow().dispatch();
    assert(!dispatch.mf_RegisterArray && !dispatch.mf_UnregisterArray);

    dispatch.mf_RegisterArray = reg;
    dispatch.mf_UnregisterArray = unreg;
}

