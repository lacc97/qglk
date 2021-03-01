#include <QFileDialog>

#include "glk.hpp"

#include "qglk.hpp"

#include "file/fileref.hpp"
#include "log/log.hpp"
#include "thread/taskrequest.hpp"

constexpr auto fn_ext = [](glui32 usg) -> std::string_view {
    using namespace std::string_view_literals;

    switch(usg & 0x03) {
        case qglk::file_reference::eSavedGame:
            return "glksave"sv;

        case qglk::file_reference::eData:
            return "glkdata"sv;

        default:
            return "txt"sv;
    }
};

frefid_t glk_fileref_create_temp(glui32 usage, glui32 rock) {
    static glui32 s_TempCounter = 0;


    std::error_code ec;
    auto tmp = std::filesystem::temp_directory_path(ec);
    if(ec && std::filesystem::exists(tmp) && std::filesystem::is_directory(tmp)) {
        auto tmpFile = tmp / fmt::format("temp{0}-{1}.{2}", s_TempCounter++, rand(), fn_ext(usage));
        if(std::filesystem::exists(tmpFile)) {
            spdlog::error("Cannot create temporary fileref because path already exists: {0}", tmpFile);
            SPDLOG_TRACE("glk_fileref_create_temp({}, {}) => {}", usage, rock, wrap::ptr(nullptr));
            return NULL;
        }

        auto* fref = new qglk::file_reference(rock);
        fref->init(tmpFile, usage);
        SPDLOG_TRACE("glk_fileref_create_temp({}, {}) => {}", usage, rock, wrap::ptr(fref));
        return fref;
    }

    SPDLOG_TRACE("glk_fileref_create_temp({}, {}) => {}", usage, rock, wrap::ptr(nullptr));
    return NULL;
}

frefid_t glk_fileref_create_by_prompt(glui32 usage, glui32 fmode, glui32 rock) {
    QString ffilter;

    switch(usage & 0x3) {
        case fileusage_SavedGame:
            ffilter = QObject::tr("Save data(*.glksave)");
            break;

        case fileusage_Data:
            ffilter = QObject::tr("Data(*.glkdata)");
            break;

        case fileusage_Transcript:
        case fileusage_InputRecord:
            ffilter = QObject::tr("Text(*.txt)");
            break;
    }

    ffilter += ";;" + QObject::tr("All files(*)");

    QString fname;
    Glk::sendTaskToEventThread([&] {
        switch(fmode) {
            case filemode_Read:
                fname = QFileDialog::getOpenFileName(&QGlk::getMainWindow(), QObject::tr("Open File"), QString(), ffilter);
                break;

            case filemode_Write:
                fname = QFileDialog::getSaveFileName(&QGlk::getMainWindow(), QObject::tr("Save File"), QString(), ffilter);
                break;

            case filemode_ReadWrite:
                fname = QFileDialog::getSaveFileName(&QGlk::getMainWindow(), QObject::tr("Modify File"), QString(), ffilter);
                break;

            case filemode_WriteAppend:
                fname = QFileDialog::getSaveFileName(&QGlk::getMainWindow(), QObject::tr("Modify File"), QString(), ffilter);
                break;
        }
    });

    if(fname.isNull()) {
        SPDLOG_TRACE("glk_fileref_create_by_prompt({}, {}, {}) => {}", usage, wrap::filemode(fmode), rock, wrap::ptr(nullptr));
        return NULL;
    }

    auto fref = new qglk::file_reference{rock};
    std::u16string_view filePath{reinterpret_cast<const char16_t*>(fname.utf16()), static_cast<size_t>(fname.size())};
    fref->init(filePath, usage);
    SPDLOG_TRACE("glk_fileref_create_by_prompt({}, {}, {}) => {}", usage, wrap::filemode(fmode), rock, wrap::ptr(fref));
    return fref;
}

frefid_t glk_fileref_create_by_name(glui32 usage, char* name, glui32 rock) {
    std::string filename{name};

    /* some sanitizing (following the glk spec recommendations) */
    filename.erase(std::remove_if(filename.begin(), filename.end(), [](char ch) -> bool {
        return (ch == '/') || (ch == '<') || (ch == '>') || (ch == ':') || (ch == '\'') ||
               (ch == '"') || (ch == '|') || (ch == '?') || (ch == '*');
    }), filename.end());
    filename.erase(std::find(filename.begin(), filename.end(), '.'), filename.end());
    if(filename.empty())
        filename = "null";


    auto namePath = std::filesystem::path{filename}.replace_extension(fn_ext(usage));
    if(std::filesystem::exists(namePath) && !std::filesystem::is_regular_file(namePath)) {
        spdlog::error("Cannot create fileref because path already exists and is not regular file: {0}", namePath);
        SPDLOG_TRACE("glk_fileref_create_by_name({}, {}, {}) => {}", usage, QString(name), rock, wrap::ptr(nullptr));
        return NULL;
    }

    auto* fref = new qglk::file_reference{rock};
    fref->init(namePath, usage);
    SPDLOG_TRACE("glk_fileref_create_by_name({}, {}, {}) => {}", usage, QString(name), rock, wrap::ptr(fref));
    return fref;
}

frefid_t glk_fileref_create_from_fileref(glui32 usage, frefid_t fref, glui32 rock) {
    auto new_fref = new qglk::file_reference(rock);
    new_fref->init(fref, usage);
    return new_fref;
}

void glk_fileref_destroy(frefid_t fref) {
    SPDLOG_TRACE("glk_fileref_destroy({})", wrap::ptr(fref));
    fref->destroy();
    delete fref;
}

frefid_t glk_fileref_iterate(frefid_t fref, glui32* rockptr) {
    const auto& frefList = QGlk::getMainWindow().fileReferenceList();

    if(fref == NULL) {
        if(frefList.empty()) {
            SPDLOG_TRACE("glk_fileref_iterate({}, {}) => {}", wrap::ptr(fref), wrap::ptr(rockptr), wrap::ptr(nullptr));
            return NULL;
        }

        auto first = frefList.front();

        if(rockptr)
            *rockptr = first->get_rock();

        SPDLOG_TRACE("glk_fileref_iterate({}, {}) => {}", wrap::ptr(fref), wrap::ptr(rockptr), wrap::ptr(first));
        return first;
    }

    auto it = frefList.cbegin();

    while(it != frefList.cend() && (*it++) != fref);

    if(it == frefList.cend()) {
        SPDLOG_TRACE("glk_fileref_iterate({}, {}) => {}", wrap::ptr(fref), wrap::ptr(rockptr), wrap::ptr(nullptr));
        return NULL;
    }

    if(rockptr)
        *rockptr = (*it)->get_rock();

    SPDLOG_TRACE("glk_fileref_iterate({}, {}) => {}", wrap::ptr(fref), wrap::ptr(rockptr), wrap::ptr(*it));
    return *it;
}

glui32 glk_fileref_get_rock(frefid_t fref) {
    return fref->get_rock();
}

void glk_fileref_delete_file(frefid_t fref) {
    SPDLOG_TRACE("glk_fileref_delete_file({})", wrap::ptr(fref));
    fref->remove();
}

glui32 glk_fileref_does_file_exist(frefid_t fref) {
    SPDLOG_TRACE("glk_fileref_does_file_exist({}) => {}", wrap::ptr(fref), FROM_FREFID(fref)->exists());
    return fref->exists();
}
