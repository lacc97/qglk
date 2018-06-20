#ifndef NULLDEVICE_HPP
#define NULLDEVICE_HPP

#include <QIODevice>

namespace Glk {
    class NullDevice : public QIODevice {
        public:
            NullDevice();
            
        protected:
            qint64 readData(char* data, qint64 maxlen) override;
            qint64 writeData(const char* data, qint64 len) override;
    };
}

#endif
