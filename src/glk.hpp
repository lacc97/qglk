#ifndef GLK_HPP
#define GLK_HPP

#include <cstddef>

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

    namespace Dispatch {
        void registerObject(Object* ptr);

        [[nodiscard]] gidispatch_rock_t dispatchRock(Object* ptr);

        void unregisterObject(Object* ptr);

//         gidispatch_rock_t arrayRock(void* ptr);
        void registerArray(void* ptr, glui32 len, bool unicode);

        void unregisterArray(void* ptr, glui32 len, bool unicode);
    }

    class Object {
            Q_DISABLE_COPY(Object)

            friend void Dispatch::registerObject(Object* ptr);

            friend gidispatch_rock_t Dispatch::dispatchRock(Object* ptr);

            friend void Dispatch::unregisterObject(Object* ptr);

        public:
            enum Type : glui32 {
                    Window = 0, Stream = 1, FileReference = 2, SoundChannel = 3
            };

            virtual ~Object() = default;

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
