#include "nulldevice.hpp"

#include <cstring>

Glk::NullDevice::NullDevice() : QIODevice() {
}


qint64 Glk::NullDevice::readData(char* data, qint64 maxlen) {
    std::memset(data, 0, maxlen);

    return maxlen;
}

qint64 Glk::NullDevice::writeData(const char* data, qint64 len) {
    return len;
}

