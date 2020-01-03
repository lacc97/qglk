#include "glk.hpp"

extern "C" {
#include "glkstart.h"
}

#include <QDir>
#include <QFileInfo>

#include "log/log.hpp"

#include "stream/latin1stream.hpp"

void glkunix_set_base_file(char* filename) {
    SPDLOG_TRACE("glkunix_set_base_file({0})", filename);

    QFileInfo fi(filename);

    QDir::setCurrent(fi.absoluteDir().absolutePath());
}

strid_t glkunix_stream_open_pathname(char* pathname, glui32 textmode, glui32 rock) {
    SPDLOG_TRACE("glkunix_stream_open_pathname({0}, {1}, {2})", pathname, (bool)textmode, rock);

    QFileInfo path(pathname);

    if(!path.exists() || path.isDir())
        return NULL;

    QIODevice::OpenMode om = QIODevice::ReadOnly;
    if(textmode)
        om |= QIODevice::Text;

    Glk::Stream* str = new Glk::Latin1Stream(NULL, new QFile(path.absoluteFilePath()), Glk::Stream::Type::File, rock);
    if(!str->open(om)) {
        delete str;
        return NULL;
    }

    return TO_STRID(str);
}
