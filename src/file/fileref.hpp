#ifndef FILEREF_HPP
#define FILEREF_HPP

#include <filesystem>

#include <fmt/format.h>
#include <fmt/printf.h>

#include <glk.hpp>

#include "object.hpp"
#include "log/format.hpp"

namespace qglk {
    using file_reference = glk_fileref_struct;
}

struct glk_fileref_struct final : public qglk::object {
  public:
    enum usage {
        eSavedGame = fileusage_SavedGame,
        eTranscript = fileusage_Transcript,
        eInputRecord = fileusage_InputRecord,
        eData = fileusage_Data,

        eBinaryMode = fileusage_BinaryMode,
        eTextMode = fileusage_TextMode
    };

    explicit glk_fileref_struct(glui32 rock) : qglk::object{rock, qglk::object::eFileReference} {}

    void init(const std::filesystem::path& path, glui32 usage) noexcept;
    void init(frefid_t fref, glui32 usage) noexcept;
    void destroy() noexcept;

    [[nodiscard]] const std::filesystem::path& get_path() const noexcept {
        return m_path;
    }
    [[nodiscard]] glui32 get_usage() const noexcept {
        return m_usage;
    }

    [[nodiscard]] bool exists() const;
    void remove() const;

  private:
    glui32 m_usage;
    std::filesystem::path m_path;
};

namespace fmt {
    template <>
    struct formatter<qglk::file_reference> {
        template <typename ParseContext>
        constexpr auto parse(ParseContext &ctx) {
            return ctx.begin();
        }

        template <typename FormatContext>
        auto format(const qglk::file_reference& w, FormatContext &ctx) {
            return format_to(ctx.out(), "{} ({})", w.get_path(), fmt::ptr(&w));
        }
    };
}

#endif
