#ifndef GLK_HPP
#define GLK_HPP

#include <cstddef>

#include <QByteArray>

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

namespace Glk {
    class Object;
    
    namespace Dispatch {
        void registerObject(Object* ptr);
        gidispatch_rock_t objectRock(Object* ptr);
        void unregisterObject(Object* ptr);
        
//         gidispatch_rock_t arrayRock(void* ptr);
        void registerArray(void* ptr, glui32 len, bool unicode);
        void unregisterArray(void* ptr, glui32 len, bool unicode);
    }
    
    class Object {
        friend void Dispatch::registerObject(Object* ptr);
        friend gidispatch_rock_t Dispatch::objectRock(Object* ptr);
        friend void Dispatch::unregisterObject(Object* ptr);
        public:
            enum class Type : glui32 {
                Window = 0, Stream = 1, FileReference = 2, SoundChannel = 3
            };
            
            Object(glui32 rock_ = 0) : m_Rock(rock_) {}
            virtual ~Object() {}

            virtual Type objectType() const = 0;
            
            inline glui32 rock() const {
                return m_Rock;
            }

        private:
            glui32 m_Rock;
            gidispatch_rock_t m_DispatchRock;
    };
}

#endif
