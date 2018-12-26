#include "glk.hpp"

#include "window/window.hpp"

glui32 glk_gestalt(glui32 id, glui32 val) {
    return glk_gestalt_ext(id, val, NULL, 0);
}


glui32 glk_gestalt_ext(glui32 id, glui32 val, glui32* arr, glui32 arrlen) {
    switch(id) {
        case gestalt_CharInput:
            switch(val) {
                case keycode_Delete:
                case keycode_Down:
                case keycode_End:
                case keycode_Escape:
                case keycode_Func1:
                case keycode_Func2:
                case keycode_Func3:
                case keycode_Func4:
                case keycode_Func5:
                case keycode_Func6:
                case keycode_Func7:
                case keycode_Func8:
                case keycode_Func9:
                case keycode_Func10:
                case keycode_Func11:
                case keycode_Func12:
                case keycode_Home:
                case keycode_Left:
                case keycode_PageDown:
                case keycode_PageUp:
                case keycode_Return:
                case keycode_Right:
                case keycode_Tab:
                case keycode_Up:
                    return TRUE;

                default:
                    return (val >= 32 && val <= 126);
            }
        
        case gestalt_CharOutput:
            if(QChar::isPrint(val))
                return gestalt_CharOutput_ExactPrint;
            else
                return gestalt_CharOutput_CannotPrint;

        case gestalt_DateTime:
            return TRUE;

        case gestalt_DrawImage:
            if(val == Glk::Window::Graphics)
                return TRUE;
            else // TODO implement for text buffer windows
                return FALSE;

        case gestalt_Graphics:
            return TRUE;

        case gestalt_GraphicsCharInput:
            return FALSE; // TODO implement

        case gestalt_GraphicsTransparency:
            return TRUE;

        case gestalt_Hyperlinks:
            return TRUE;

        case gestalt_HyperlinkInput:
            switch(val) {
//                 case Glk::Window::Graphics:
                case Glk::Window::TextBuffer:
                    return TRUE;

                default:
                    return FALSE;
            }

        case gestalt_LineInput:
            return (val >= 32 && val <= 126);

        case gestalt_LineInputEcho:
            return TRUE;

        case gestalt_LineTerminators:
            return TRUE;

        case gestalt_LineTerminatorKey:
            switch(val) {
                case keycode_End:
                case keycode_Escape:
                case keycode_Func1:
                case keycode_Func2:
                case keycode_Func3:
                case keycode_Func4:
                case keycode_Func5:
                case keycode_Func6:
                case keycode_Func7:
                case keycode_Func8:
                case keycode_Func9:
                case keycode_Func10:
                case keycode_Func11:
                case keycode_Func12:
                case keycode_Home:
                case keycode_PageDown:
                case keycode_PageUp:
                case keycode_Tab:
                    return TRUE;
            }

            return FALSE;

        case gestalt_MouseInput:
            switch(val) {
                case wintype_Graphics:
                case wintype_TextGrid:
                    return TRUE;
            }

            return FALSE;

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
