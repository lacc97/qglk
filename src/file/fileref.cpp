#include "fileref.hpp"

#include <cassert>

#include "qglk.hpp"

#include "log/log.hpp"

Glk::FileReference::FileReference(const QFileInfo& fi_, glui32 usage_, glui32 rock_) : Object(rock_), m_FileInfo(fi_), m_Usage(usage_) {
    Q_ASSERT(!m_FileInfo.isDir());

    QGlk::getMainWindow().dispatch().registerObject(this);
    QGlk::getMainWindow().fileReferenceList().push_back(this);
}

Glk::FileReference::FileReference(const Glk::FileReference& fref_, glui32 usage_, glui32 rock_) : Object(rock_), m_FileInfo(fref_.m_FileInfo), m_Usage(usage_) {
    Q_ASSERT(!m_FileInfo.isDir());

    QGlk::getMainWindow().dispatch().registerObject(this);
    QGlk::getMainWindow().fileReferenceList().push_back(this);
}

Glk::FileReference::~FileReference() {
    auto& frfList = QGlk::getMainWindow().fileReferenceList();
    if(std::count(frfList.begin(), frfList.end(), this) == 0) {
        spdlog::warn("File reference {} not found in file reference list while removing", *this);
    } else {
        frfList.remove(this);
        SPDLOG_TRACE("File reference {} removed from file reference list", *this);
    }

    QGlk::getMainWindow().dispatch().unregisterObject(this);
}

QString Glk::FileReference::path() const {
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
