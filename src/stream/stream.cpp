#include "stream.hpp"

#include <cassert>
#include <cstring>

#include <QBuffer>
#include <QDebug>

#include "qglk.hpp"
#include "blorb/chunk.hpp"

Glk::Stream::Stream(QObject* parent_, QIODevice* device_, Glk::Stream::Type type_, bool unicode_, void* userptr_, glui32 rock_) : QObject(parent_), Object(rock_), mp_Device(device_), m_Unicode(unicode_), m_ReadChars(0), m_WriteChars(0), m_Type(type_), mp_ExtraData(userptr_) {
    assert(mp_Device);
}

Glk::Stream::~Stream() {
    if(mp_Device->isOpen()) {
        mp_Device->close();

        emit closed();

        if(!QGlk::getMainWindow().streamList().removeOne(this))
            qWarning() << "this" << (this) << "not found in stream list while removing";

        Glk::Dispatch::unregisterObject(this);
    }
    
    switch(type()) {
        case Type::Memory:
            if(data()) {
                QByteArray& qba = static_cast<QBuffer*>(mp_Device)->buffer();

                if((device()->openMode() & QIODevice::WriteOnly) != 0)
                    std::memcpy(data(), qba.data(), qba.size());

                Glk::Dispatch::unregisterArray(data(), (isUnicode() ? qba.size() / 4 : qba.size()), isUnicode());

                delete &static_cast<QBuffer*>(mp_Device)->buffer();
            }

            break;

        case Type::Resource:
            Glk::Blorb::unloadChunk(*reinterpret_cast<Glk::Blorb::Chunk*>(data()));
            delete reinterpret_cast<Glk::Blorb::Chunk*>(data());
            break;
    }

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
    }

    return done;
}

void Glk::Stream::pushStyle(Style::Type sty) {}

bool Glk::Stream::isOpen() const {
    return mp_Device->isOpen();
}

#include "moc_stream.cpp"





