#include "dispatch.hpp"

#include <spdlog/spdlog.h>

#include "qglk.hpp"

void Glk::Dispatch::registerObject(Glk::Object* ptr) {
    if(mf_RegisterObject) {
        switch(ptr->objectType()) {
            case Glk::Object::Type::Window:
                assert(dynamic_cast<Glk::Window*>(ptr));
                ptr->m_DispatchRock = mf_RegisterObject(static_cast<Glk::Window*>(ptr), glui32(ptr->objectType()));
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

void Glk::Dispatch::registerObject(qglk::object* ptr) {
    if(!mf_RegisterObject)
        return;

    switch(ptr->get_type()) {
        case qglk::object::eStream:
            assert(dynamic_cast<qglk::stream*>(ptr));
            ptr->m_dispatch_rock = mf_RegisterObject(static_cast<qglk::stream*>(ptr), static_cast<glui32>(ptr->get_type()));
            return;

        case qglk::object::eFileReference:
            assert(dynamic_cast<qglk::file_reference*>(ptr));
            ptr->m_dispatch_rock = mf_RegisterObject(static_cast<qglk::file_reference*>(ptr), static_cast<glui32>(ptr->get_type()));
            return;

        default:
            spdlog::error("Object({0}) has an invalid object type: {1}", fmt::ptr(ptr), static_cast<glui32>(ptr->get_type()));
            glk_exit();
    }
}

void Glk::Dispatch::unregisterObject(Glk::Object* ptr) {
    if(mf_UnregisterObject)
        switch(ptr->objectType()) {
            case Glk::Object::Type::Window:
                assert(dynamic_cast<Glk::Window*>(ptr));
                mf_UnregisterObject(static_cast<Glk::Window*>(ptr), glui32(ptr->objectType()), ptr->m_DispatchRock);
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

void Glk::Dispatch::unregisterObject(qglk::object* ptr) {
    if(!mf_UnregisterObject)
        return;

    switch(ptr->get_type()) {
        case qglk::object::eStream:
            assert(dynamic_cast<qglk::stream*>(ptr));
            mf_UnregisterObject(static_cast<qglk::stream*>(ptr), glui32(ptr->get_type()), ptr->get_dispatch_rock());
            return;

        case qglk::object::eFileReference:
            assert(dynamic_cast<qglk::file_reference*>(ptr));
            mf_UnregisterObject(static_cast<qglk::stream*>(ptr), glui32(ptr->get_type()), ptr->get_dispatch_rock());
            return;

        default:
            spdlog::error("Object({0}) has an invalid object type: {1}", fmt::ptr(ptr), static_cast<glui32>(ptr->get_type()));
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