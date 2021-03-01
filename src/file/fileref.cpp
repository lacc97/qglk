#include "fileref.hpp"

#include <cassert>

#include "qglk.hpp"

#include "log/log.hpp"

void glk_fileref_struct::destroy() noexcept {
    QGlk::getMainWindow().fileReferenceList().remove(this);

    destroy_base();
}

bool glk_fileref_struct::exists() const {
    return std::filesystem::exists(m_path);
}

void glk_fileref_struct::init(const std::filesystem::path& path, glui32 usage) noexcept {
    init_base();

    QGlk::getMainWindow().fileReferenceList().push_back(this);

    m_path = path;
    m_usage = usage;
}

void glk_fileref_struct::init(frefid_t fref, glui32 usage) noexcept {
    assert(fref);

    init_base();

    QGlk::getMainWindow().fileReferenceList().push_back(this);

    m_path = fref->get_path();
    m_usage = usage;
}

void glk_fileref_struct::remove() const {
    std::filesystem::remove(m_path);
}
