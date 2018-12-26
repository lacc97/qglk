#include "fileref.hpp"

#include <cassert>

Glk::FileReference::FileReference(const QFileInfo& fi_, glui32 usage_, glui32 rock_) : Object(rock_), m_FileInfo(fi_), m_Usage(usage_) {
    Q_ASSERT(!m_FileInfo.isDir());

    s_FileReferenceSet.insert(this);
    Glk::Dispatch::registerObject(this);
}

Glk::FileReference::FileReference(const Glk::FileReference& fref_, glui32 usage_, glui32 rock_) : Object(rock_), m_FileInfo(fref_.m_FileInfo), m_Usage(usage_) {
    Q_ASSERT(!m_FileInfo.isDir());

    s_FileReferenceSet.insert(this);
    Glk::Dispatch::registerObject(this);
}

Glk::FileReference::~FileReference() {
    Glk::Dispatch::unregisterObject(this);
    s_FileReferenceSet.remove(this);
}

QString Glk::FileReference::path() {
    return m_FileInfo.absolutePath();
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
