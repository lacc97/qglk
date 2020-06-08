#ifndef GLK_HPP
#define GLK_HPP

#include <cstddef>

#include <map>
#include <sstream>

#include <QByteArray>
#include <QMetaType>
#include <QString>

#ifndef TRUE
#define TRUE (1)
#endif

#ifndef FALSE
#define FALSE (0)
#endif

extern "C" {
#include "glk.h"
#include "gi_blorb.h"
#include "gi_dispa.h"
}

// Q_DECLARE_METATYPE(glui32)

namespace Glk {
    class Object;

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

            void unregisterObject(Object* ptr);

        private:
            ObjectRegisterFunction* mf_RegisterObject{nullptr};
            ObjectUnregisterFunction* mf_UnregisterObject{nullptr};

            std::map<void*, gidispatch_rock_t> m_ArrayRegistry{};
            ArrayRegisterFunction* mf_RegisterArray{nullptr};
            ArrayUnregisterFunction* mf_UnregisterArray{nullptr};
    };

    class Object {
            Q_DISABLE_COPY(Object)

            friend class Dispatch;

        public:
            enum Type : glui32 {
                    Window = 0, Stream = 1, FileReference = 2, SoundChannel = 3
            };

            virtual ~Object() = default;


            [[nodiscard]] inline gidispatch_rock_t dispatchRock() const {
                return m_DispatchRock;
            }

            [[nodiscard]] virtual Type objectType() const = 0;

            [[nodiscard]] inline glui32 rock() const {
                return m_Rock;
            }

        protected:
            explicit Object(glui32 rock_ = 0) : m_Rock{rock_} {}

        private:
            glui32 m_Rock;
            gidispatch_rock_t m_DispatchRock;
    };
}

template<typename T>
inline const std::string to_string(T val) {
    std::ostringstream ss;
    ss << val;
    return ss.str();
}

#endif
