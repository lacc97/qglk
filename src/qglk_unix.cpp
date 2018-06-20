#include "glk.hpp"

extern "C" {
#include "glkstart.h"
}

#include <QDir>
#include <QFileInfo>

#include "file/fileref.hpp"

void glkunix_set_base_file(char* filename) {
    QFileInfo fi(filename);
    
    QDir::setCurrent(fi.absoluteDir().absolutePath());
}

strid_t glkunix_stream_open_pathname(char* pathname, glui32 textmode, glui32 rock) {
    Glk::FileReference* fref = new Glk::FileReference(QFileInfo(pathname), (textmode != 0 ? Glk::FileReference::TextMode : Glk::FileReference::BinaryMode), 0);
    
    strid_t str = glk_stream_open_file(TO_FREFID(fref), filemode_Read, rock);
    
    delete fref;
    
    return str;
}
