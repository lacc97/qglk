#ifndef LOG_HPP
#define LOG_HPP

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <QString>

#define DOTRACE

namespace Log {
    extern const uint64_t StartTime;

    enum Level {
        TRACE, DEBUG, INFO, WARNING, ERROR, FATAL
    };

    template <typename CharT, std::basic_ostream<CharT>& ErrorStream>
    class BasicStream : public std::basic_ostream<CharT> {
        public:
            BasicStream(Level lvl) : std::basic_ostream<CharT>(new std::basic_stringbuf<CharT>()), m_Level(lvl) {}
            ~BasicStream() {
                constexpr int width = 9;
                constexpr CharT fillch = '0';

                auto nowDate = std::chrono::high_resolution_clock::now();
                auto milli = uint64_t(nowDate.time_since_epoch().count() * double(std::chrono::high_resolution_clock::period::num) / double(std::chrono::high_resolution_clock::period::den) * 1000) - StartTime;
                auto buf = static_cast<std::basic_stringbuf<CharT>*>(this->rdbuf())->str();

                std::basic_stringstream<CharT> ss;
                ss << std::setw(width) << std::setfill(fillch) << milli;
                
                switch(m_Level) {
                    case TRACE:
                        ss << ": [TRA] ";
                        break;

                    case DEBUG:
                        ss << ": [DEB] ";
                        break;

                    case INFO:
                        ss << ": [INF] ";
                        break;

                    case WARNING:
                        ss << ": [WRN] ";
                        break;

                    case ERROR:
                        ss << ": [ERR] ";
                        break;

                    case FATAL:
                        ss << ": [FAT] ";
                        break;
                }
                
                ss << buf << std::endl;
                
                ErrorStream << ss.str();

                delete this->rdbuf();
            }

        private:
            Level m_Level;
    };
    
#ifdef NDEBUG
    template <typename CharT>
    class NullStream : public std::basic_ostream<CharT> {};
    
    extern template class NullStream<char>;
    extern template class NullStream<wchar_t>;
#endif
    
    extern template class BasicStream<char, std::clog>;
    typedef BasicStream<char, std::clog> Stream;
    
    extern template class BasicStream<wchar_t, std::wclog>;
    typedef BasicStream<wchar_t, std::wclog> WStream;
}

#ifdef NDEBUG
#include <type_traits>

template <typename CharT, class T>
inline Log::NullStream<CharT>& operator<<(Log::NullStream<CharT>& nos, typename std::conditional<std::is_fundamental<T>::value, T, const T&>::type t) {
    if(false)
        static_cast<std::basic_ostream<CharT>>(nos) << t;
    
    return nos;
}
#endif

template <typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const QString& str);

#ifndef NDEBUG
#ifdef DOTRACE
#define trace() Log::Stream(Log::TRACE)
#else
#define trace() Log::NullStream<char>()
#endif
#define debug() Log::Stream(Log::DEBUG)
#else
#define trace() Log::NullStream<char>()
#define debug() Log::NullStream<char>()
#endif

#define info()  Log::Stream(Log::INFO)
#define warn()  Log::Stream(Log::WARNING)
#define error() Log::Stream(Log::ERROR)
#define fatal() Log::Stream(Log::FATAL)

#endif
