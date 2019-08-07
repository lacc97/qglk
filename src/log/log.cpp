#include "log.hpp"

#include <QDebug>
#include <QStringRef>

const uint64_t Log::StartTime = uint64_t(std::chrono::high_resolution_clock::now().time_since_epoch().count() * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den) * 1000);

//template class Log::NullStream<char>;
//template class Log::NullStream<wchar_t>;

template class Log::BasicStream<char, std::clog>;
template class Log::BasicStream<wchar_t, std::wclog>;

QString toPrintable(const QString& str) {
    QString out;

    QDebug* deb = new QDebug(&out);
    (*deb) << str;
    delete deb;

    out.chop(1);

    return out;
}

template<> std::basic_ostream<char>& operator<<(std::basic_ostream<char>& os, const QString& str) {
    return os << toPrintable(str).toStdString();
}

template<> std::basic_ostream<wchar_t>& operator<<(std::basic_ostream<wchar_t>& os, const QString& str) {
    return os << toPrintable(str).toStdWString();
}
