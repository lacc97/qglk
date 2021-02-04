#ifndef QGLK_DISPATCH_HPP
#define QGLK_DISPATCH_HPP

#include <glk.hpp>

#include "object.hpp"

namespace Glk {
    class Dispatch {
        using ObjectRegisterFunction = gidispatch_rock_t(void*, glui32);
        using ObjectUnregisterFunction = void(void*, glui32, gidispatch_rock_t);
        using ArrayRegisterFunction = gidispatch_rock_t(void*, glui32, char*);
        using ArrayUnregisterFunction = void(void*, glui32, char*, gidispatch_rock_t);

        friend void ::gidispatch_set_object_registry(ObjectRegisterFunction* reg, ObjectUnregisterFunction* unreg);
        friend void ::gidispatch_set_retained_registry(ArrayRegisterFunction* reg, ArrayUnregisterFunction* unreg);

      public:
        void registerArray(void* ptr, glui32 len, bool unicode);
        void unregisterArray(void* ptr, glui32 len, bool unicode);

        void registerObject(Object* ptr);
        void registerObject(qglk::object* ptr);

        void unregisterObject(Object* ptr);
        void unregisterObject(qglk::object* ptr);

      private:
        ObjectRegisterFunction* mf_RegisterObject{nullptr};
        ObjectUnregisterFunction* mf_UnregisterObject{nullptr};

        std::map<void*, gidispatch_rock_t> m_ArrayRegistry{};
        ArrayRegisterFunction* mf_RegisterArray{nullptr};
        ArrayUnregisterFunction* mf_UnregisterArray{nullptr};
    };
}

#endif    //QGLK_DISPATCH_HPP
