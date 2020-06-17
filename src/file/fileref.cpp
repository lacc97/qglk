#include "fileref.hpp"

#include <cassert>

#include "qglk.hpp"

#include "log/log.hpp"


Glk::FileReference::FileReference(const std::filesystem::path& path_, glui32 usage_, glui32 rock_)
        : Object(rock_),
          m_Path(std::filesystem::absolute(path_)),
          m_Usage(usage_) {
    Q_ASSERT(!std::filesystem::is_directory(m_Path));

    QGlk::getMainWindow().dispatch().registerObject(this);
    QGlk::getMainWindow().fileReferenceList().push_back(this);
}

Glk::FileReference::FileReference(const Glk::FileReference& fref_, glui32 usage_, glui32 rock_)
        : Object(rock_),
          m_Path(fref_.m_Path),
          m_Usage(usage_) {
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

bool Glk::FileReference::exists() const {
    return std::filesystem::exists(m_Path);
}

void Glk::FileReference::remove() const {
    std::filesystem::remove(m_Path);
}

glui32 Glk::FileReference::usage() const {
    return m_Usage;
}
