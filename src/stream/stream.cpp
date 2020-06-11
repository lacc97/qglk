#include "stream.hpp"

#include <cassert>
#include <cstring>

#include <string_view>

#include "glk.hpp"
#include "qglk.hpp"
#include "blorb/chunk.hpp"

#include "log/log.hpp"

Glk::Stream::Stream(QObject* parent_, std::unique_ptr<std::streambuf> buf_, Type type_, bool text_, bool unicode_, glui32 rock_)
        : QObject{parent_},
          m_Type{type_},
          m_TextMode{text_},
          m_Unicode{unicode_},
          mp_Streambuf{std::move(buf_)} {
    assert(mp_Streambuf);
    {
        QGlk::getMainWindow().dispatch().registerObject(this);
        QGlk::getMainWindow().streamList().push_back(this);
        SPDLOG_DEBUG("Stream {} appended to stream list", *this);
    }
}

Glk::Stream::~Stream() {
    emit closed();

    {
        auto& strList = QGlk::getMainWindow().streamList();
        if(std::count(strList.begin(), strList.end(), this) == 0) {
            spdlog::warn("Stream {} not found in stream list while removing", *this);
        } else {
            strList.remove(this);
            SPDLOG_DEBUG("Stream {} removed from stream list", *this);
        }

        QGlk::getMainWindow().dispatch().unregisterObject(this);
    }

    if(glk_stream_get_current() == TO_STRID(this))
        glk_stream_set_current(NULL);
}

Glk::Object::Type Glk::Stream::objectType() const {
    return Object::Type::Stream;
}

void Glk::Stream::pushStyle(Style::Type sty) {}
