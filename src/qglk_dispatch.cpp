#include "glk.hpp"

#include <QDebug>
#include <QSet>
#include <QHash>

#include "file/fileref.hpp"
#include "sound/schannel.hpp"
#include "stream/stream.hpp"
#include "window/window.hpp"

gidispatch_rock_t (* s_ObjectRegisterFunction)(void* obj, glui32 objclass) = NULL;

void (* s_ObjectUnregisterFunction)(void* obj, glui32 objclass, gidispatch_rock_t objrock) = NULL;

QHash<void*, gidispatch_rock_t> s_ArrayHash;

gidispatch_rock_t (* s_ArrayRegisterFunction)(void* array, glui32 len, char* typecode) = NULL;

void (* s_ArrayUnregisterFunction)(void* array, glui32 len, char* typecode, gidispatch_rock_t objrock) = NULL;

void Glk::Dispatch::registerObject(Glk::Object* ptr) {
    if(s_ObjectRegisterFunction) {
        switch(ptr->objectType()) {
            case Glk::Object::Type::Window:
                ptr->m_DispatchRock = s_ObjectRegisterFunction(static_cast<Glk::Window*>(ptr),
                                                               glui32(ptr->objectType()));
                return;

            case Glk::Object::Type::Stream:
                ptr->m_DispatchRock = s_ObjectRegisterFunction(static_cast<Glk::Stream*>(ptr),
                                                               glui32(ptr->objectType()));
                return;

            case Glk::Object::Type::FileReference:
                ptr->m_DispatchRock = s_ObjectRegisterFunction(static_cast<Glk::FileReference*>(ptr),
                                                               glui32(ptr->objectType()));
                return;

            case Glk::Object::Type::SoundChannel:
                ptr->m_DispatchRock = s_ObjectRegisterFunction(static_cast<Glk::SoundChannel*>(ptr),
                                                               glui32(ptr->objectType()));
                return;

            default:
                qCritical() << "Object" << (ptr) << "has an invalid object type" << glui32(ptr->objectType());
                glk_exit();
        }
    }
}

gidispatch_rock_t Glk::Dispatch::dispatchRock(Glk::Object* ptr) {
    return ptr->m_DispatchRock;
}

void Glk::Dispatch::unregisterObject(Glk::Object* ptr) {
    if(s_ObjectUnregisterFunction)
        switch(ptr->objectType()) {
            case Glk::Object::Type::Window:
                s_ObjectUnregisterFunction(static_cast<Glk::Window*>(ptr), glui32(ptr->objectType()),
                                           ptr->m_DispatchRock);
                return;

            case Glk::Object::Type::Stream:
                s_ObjectUnregisterFunction(static_cast<Glk::Stream*>(ptr), glui32(ptr->objectType()),
                                           ptr->m_DispatchRock);
                return;

            case Glk::Object::Type::FileReference:
                s_ObjectUnregisterFunction(static_cast<Glk::FileReference*>(ptr), glui32(ptr->objectType()),
                                           ptr->m_DispatchRock);
                return;

            case Glk::Object::Type::SoundChannel:
                s_ObjectUnregisterFunction(static_cast<Glk::SoundChannel*>(ptr), glui32(ptr->objectType()),
                                           ptr->m_DispatchRock);
                return;

            default:
                qCritical() << "Object" << (ptr) << "has an invalid object type" << glui32(ptr->objectType());
                glk_exit();
        }
}

// gidispatch_rock_t Glk::Dispatch::arrayRock(void* ptr) {
// }

const char CHAR_CODE[] = "&+#!Cn";
const char UCS4_CODE[] = "&+#!Iu";

void Glk::Dispatch::registerArray(void* ptr, glui32 len, bool unicode) {
    if(s_ArrayRegisterFunction)
        s_ArrayHash[ptr] = s_ArrayRegisterFunction(ptr, len, (char*) (unicode ? UCS4_CODE : CHAR_CODE));
}

void Glk::Dispatch::unregisterArray(void* ptr, glui32 len, bool unicode) {
    if(s_ArrayUnregisterFunction)
        s_ArrayUnregisterFunction(ptr, len, (char*) (unicode ? UCS4_CODE : CHAR_CODE), s_ArrayHash[ptr]);

    s_ArrayHash.remove(ptr);
}


void gidispatch_set_object_registry(gidispatch_rock_t (* reg)(void* obj, glui32 objclass),
                                    void (* unreg)(void* obj, glui32 objclass, gidispatch_rock_t objrock)) {
    s_ObjectRegisterFunction = reg;
    s_ObjectUnregisterFunction = unreg;

    if(s_ObjectRegisterFunction) {
        for(frefid_t fref = glk_fileref_iterate(NULL, NULL); fref; fref = glk_fileref_iterate(fref, NULL))
            Glk::Dispatch::registerObject(FROM_FREFID(fref));

        for(schanid_t schan = glk_schannel_iterate(NULL, NULL); schan; schan = glk_schannel_iterate(schan, NULL))
            Glk::Dispatch::registerObject(FROM_SCHANID(schan));

        for(strid_t str = glk_stream_iterate(NULL, NULL); str; str = glk_stream_iterate(str, NULL))
            Glk::Dispatch::registerObject(FROM_STRID(str));

        for(winid_t win = glk_window_iterate(NULL, NULL); win; win = glk_window_iterate(win, NULL))
            Glk::Dispatch::registerObject(FROM_WINID(win));
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

    return Glk::Dispatch::dispatchRock(glkobj);
}

void gidispatch_set_retained_registry(gidispatch_rock_t (* reg)(void* array, glui32 len, char* typecode),
                                      void (* unreg)(void* array, glui32 len, char* typecode,
                                                     gidispatch_rock_t objrock)) {
    s_ArrayRegisterFunction = reg;
    s_ArrayUnregisterFunction = unreg;
}

