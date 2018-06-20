#include "glk.hpp"

#include <QHash>

gidispatch_rock_t (*s_ObjectRegisterFunction)(void* obj, glui32 objclass) = NULL;
void (*s_ObjectUnregisterFunction)(void* obj, glui32 objclass, gidispatch_rock_t objrock) = NULL;

QHash<void*, gidispatch_rock_t> s_ArrayHash;
gidispatch_rock_t (*s_ArrayRegisterFunction)(void* array, glui32 len, char* typecode) = NULL;
void (*s_ArrayUnregisterFunction)(void* array, glui32 len, char* typecode, gidispatch_rock_t objrock) = NULL;


void Glk::Dispatch::registerObject(Glk::Object* ptr) {
    if(s_ObjectRegisterFunction)
        ptr->m_DispatchRock = s_ObjectRegisterFunction(ptr, glui32(ptr->objectType()));
}

gidispatch_rock_t Glk::Dispatch::objectRock(Glk::Object* ptr) {
    return ptr->m_DispatchRock;
}

void Glk::Dispatch::unregisterObject(Glk::Object* ptr) {
    if(s_ObjectUnregisterFunction)
        s_ObjectUnregisterFunction(ptr, glui32(ptr->objectType()), ptr->m_DispatchRock);
}

// gidispatch_rock_t Glk::Dispatch::arrayRock(void* ptr) {
// }

const char CHAR_CODE[] = "&+#!Cn";
const char UCS4_CODE[] = "&+#!Iu";
void Glk::Dispatch::registerArray(void* ptr, glui32 len, bool unicode) {
    if(s_ArrayRegisterFunction)
        s_ArrayHash[ptr] = s_ArrayRegisterFunction(ptr, len, (char*)(unicode ? UCS4_CODE : CHAR_CODE));
}

void Glk::Dispatch::unregisterArray(void* ptr, glui32 len, bool unicode) {
    if(s_ArrayUnregisterFunction)
        s_ArrayUnregisterFunction(ptr, len, (char*)(unicode ? UCS4_CODE : CHAR_CODE), s_ArrayHash[ptr]);

    s_ArrayHash.remove(ptr);
}


void gidispatch_set_object_registry(gidispatch_rock_t (*reg)(void* obj, glui32 objclass), void (*unreg)(void* obj, glui32 objclass, gidispatch_rock_t objrock)) {
    s_ObjectRegisterFunction = reg;
    s_ObjectUnregisterFunction = unreg;
}

gidispatch_rock_t gidispatch_get_objrock(void* obj, glui32 objclass) {
    return Glk::Dispatch::objectRock(static_cast<Glk::Object*>(obj));
}

void gidispatch_set_retained_registry(gidispatch_rock_t (*reg)(void* array, glui32 len, char* typecode), void (*unreg)(void* array, glui32 len, char* typecode, gidispatch_rock_t objrock)) {
    s_ArrayRegisterFunction = reg;
    s_ArrayUnregisterFunction = unreg;
}

