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
    close();

    if(!QGlk::getMainWindow().streamList().removeOne(this))
        warn() << "Stream " << (this) << " not found in stream list while removing";
    else
        trace() << "Stream " << (this) << " removed from stream list";

    Glk::Dispatch::unregisterObject(this);

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
        Glk::Dispatch::registerObject(this);
        QGlk::getMainWindow().streamList().append(this);
        trace() << "Stream " << (this) << " appended to stream list";
    }

    return done;
}

void Glk::Stream::pushStyle(Style::Type sty) {}

bool Glk::Stream::isOpen() const {
    return mp_Device->isOpen();
}

#include "moc_stream.cpp"





