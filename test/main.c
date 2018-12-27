#include "glk.h"
#include "glkstart.h"

#include <stdio.h>

glkunix_argumentlist_t glkunix_arguments[] = {
    { NULL, glkunix_arg_End, NULL }
};

int glkunix_startup_code(glkunix_startup_t* data) {
    return TRUE;
}

void glk_main() {
    frefid_t tmp = glk_fileref_create_by_name(fileusage_SavedGame, "tmp", 0);
    strid_t ostr = glk_stream_open_file_uni(tmp, filemode_Write, 0);

    glk_stream_set_current(ostr);
    glk_put_string("Hello, World!\nThis is a test.\nXyzzy\n");
    glk_stream_close(ostr, 0);

    strid_t istr = glk_stream_open_file_uni(tmp, filemode_Read, 0);

    char buf[256];
    glk_get_line_stream(istr, buf, 256);

    printf("line 1: %s\n", buf);
    
    glk_get_line_stream(istr, buf, 256);

    printf("line 2: %s\n", buf);

    glk_stream_close(istr, 0);

    glk_fileref_destroy(tmp);
    
    glk_exit();
}
