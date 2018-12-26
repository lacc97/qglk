#include "glk.hpp"

extern "C" {
#include "glkstart.h"
}

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include "stream/latin1stream.hpp"

void glkunix_set_base_file(char* filename) {
    QFileInfo fi(filename);

    QDir::setCurrent(fi.absoluteDir().absolutePath());
}

strid_t glkunix_stream_open_pathname(char* pathname, glui32 textmode, glui32 rock) {
#ifndef NDEBUG
    QDebug deb = qDebug();
    deb << "glkunix_stream_open_pathname(" << QString(pathname) << "," << textmode << "," << rock << ") =>";
#endif

    QFileInfo path(pathname);

    if(!path.exists() || path.isDir())
        return NULL;

    Glk::Stream* str = new Glk::Latin1Stream(NULL, new QFile(path.absoluteFilePath()), Glk::Stream::Type::File, rock);
    QIODevice::OpenMode om = QIODevice::ReadOnly;

    if(!str->open(om)) {
        delete str;
#ifndef NDEBUG
        deb << ((void*)NULL);
#endif
        return NULL;
    }

#ifndef NDEBUG
    deb << TO_STRID(str);
#endif

    return TO_STRID(str);
}
