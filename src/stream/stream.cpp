#include "stream.hpp"

#include <cassert>
#include <cstring>

#include <QBuffer>

#include "glk.hpp"
#include "qglk.hpp"
#include "blorb/chunk.hpp"

#include "log/log.hpp"

Glk::Stream::Stream(QObject* parent_, QIODevice* device_, Glk::Stream::Type type_, bool unicode_, glui32 rock_) : QObject(parent_), Object(rock_), mp_Device(device_), m_Unicode(unicode_), m_ReadChars(0), m_WriteChars(0), m_Type(type_) {
    assert(mp_Device);
}

Glk::Stream::~Stream() {
    if(mp_Device->isOpen()) {
        mp_Device->close();
        if(!mp_Device->isOpen())
            emit closed();

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

    delete mp_Device;
}

Glk::Object::Type Glk::Stream::objectType() const {
    return Object::Type::Stream;
}


bool Glk::Stream::open(QIODevice::OpenMode om) {
    if(type() == Type::Memory)
        om &= (~QIODevice::Text);

    bool done = mp_Device->open(om);

    if(done) {
        QGlk::getMainWindow().dispatch().registerObject(this);
        QGlk::getMainWindow().streamList().push_back(this);
        SPDLOG_DEBUG("Stream {} appended to stream list", *this);
    }

    return done;
}

void Glk::Stream::pushStyle(Style::Type sty) {}

bool Glk::Stream::isOpen() const {
    return mp_Device->isOpen();
}





