#include "glk.hpp"

extern "C" {
#include "glkstart.h"
}

#include <filesystem>
#include <fstream>
#include <memory>

#include "log/log.hpp"

#include "stream/latin1stream.hpp"

void glkunix_set_base_file(char* filename) {
    SPDLOG_TRACE("glkunix_set_base_file({0})", filename);

    std::error_code ec;
    std::filesystem::path p = std::filesystem::absolute(filename, ec);
    if(ec) {
        spdlog::error("Failed to set current directory to '{}': {}", p.c_str(), ec.message());
        return;
    }

    if(std::filesystem::is_directory(p))
        std::filesystem::current_path(p);
    else
        std::filesystem::current_path(p.parent_path());
}

strid_t glkunix_stream_open_pathname(char* pathname, glui32 textmode, glui32 rock) {
    SPDLOG_TRACE("glkunix_stream_open_pathname({0}, {1}, {2})", pathname, (bool)textmode, rock);

    std::unique_ptr<std::filebuf> filebuf = std::make_unique<std::filebuf>();
    if(!filebuf->open(std::filesystem::path{pathname}, std::ios_base::in)) {
        spdlog::error("Failed to open file '{}'", pathname);
        return NULL;
    }

    return TO_STRID(new Glk::Latin1Stream{nullptr, std::move(filebuf), Glk::Stream::Type::File, textmode != 0, rock});
}
