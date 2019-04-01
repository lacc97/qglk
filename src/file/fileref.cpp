#include "fileref.hpp"

#include <cassert>

#include "qglk.hpp"

#include "log/log.hpp"

Glk::FileReference::FileReference(const QFileInfo& fi_, glui32 usage_, glui32 rock_) : Object(rock_), m_FileInfo(fi_), m_Usage(usage_) {
    Q_ASSERT(!m_FileInfo.isDir());

    Glk::Dispatch::registerObject(this);
    QGlk::getMainWindow().fileReferenceList().append(this);
}

Glk::FileReference::FileReference(const Glk::FileReference& fref_, glui32 usage_, glui32 rock_) : Object(rock_), m_FileInfo(fref_.m_FileInfo), m_Usage(usage_) {
    Q_ASSERT(!m_FileInfo.isDir());

    Glk::Dispatch::registerObject(this);
    QGlk::getMainWindow().fileReferenceList().append(this);
}

Glk::FileReference::~FileReference() {
    if(!QGlk::getMainWindow().fileReferenceList().removeOne(this))
        log_warn() << "File reference " << (this) << " not found in file reference list while removing";
    else
        log_trace() << "File reference " << (this) << " removed from file reference list";

    Glk::Dispatch::unregisterObject(this);
}

QString Glk::FileReference::path() {
    return m_FileInfo.absoluteFilePath();
}

bool Glk::FileReference::exists() const {
    return m_FileInfo.exists();
}

void Glk::FileReference::remove() const {
    QFile::remove(m_FileInfo.absoluteFilePath());
}

QFile* Glk::FileReference::file() const {
    return new QFile(m_FileInfo.absoluteFilePath());
}

glui32 Glk::FileReference::usage() const {
    return m_Usage;
}
