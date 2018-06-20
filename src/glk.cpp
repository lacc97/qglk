#include "glk.hpp"


void glk_tick() {}

glui32 glk_gestalt(glui32 id, glui32 val) {
    return glk_gestalt_ext(id, val, NULL, 0);
}


glui32 glk_gestalt_ext(glui32 id, glui32 val, glui32* arr, glui32 arrlen) {
    switch(id) {
        case gestalt_DateTime:
            return TRUE;
            
        case gestalt_Graphics:
            return TRUE;
            
        case gestalt_GraphicsTransparency:
            return TRUE;

        case gestalt_ResourceStream:
            return TRUE;

        case gestalt_Sound:
            return TRUE;

        case gestalt_SoundMusic:
            return TRUE;

        case gestalt_SoundVolume:
            return TRUE;

        case gestalt_Sound2:
            return TRUE;
            
        case gestalt_Timer:
            return TRUE;

        case gestalt_Unicode:
            return TRUE;

        case gestalt_UnicodeNorm:
            return TRUE;

        case gestalt_Version:
            /* This implements Glk spec version 0.7.5. */
            return 0x00000705;
    }

    return FALSE;
}
