#ifndef LOG_HPP
#define LOG_HPP

#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <type_traits>

#include <QString>


/// This header should always be included LAST.

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

//    template <typename CharT>
//    class NullStream {};
//
//    extern template class NullStream<char>;
//    extern template class NullStream<wchar_t>;
    
    extern template class BasicStream<char, std::clog>;
    typedef BasicStream<char, std::clog> Stream;
    
    extern template class BasicStream<wchar_t, std::wclog>;
    typedef BasicStream<wchar_t, std::wclog> WStream;
}

//template <typename CharT, class T>
//inline Log::NullStream<CharT>& operator<<(Log::NullStream<CharT>& nos, T t) {
//    if(false)
//        Log::BasicStream<CharT, std::clog>(Log::INFO) << t;
//
//    return nos;
//}

template <typename CharT>
std::basic_ostream<CharT>& operator<<(std::basic_ostream<CharT>& os, const QString& str);

#ifndef NDEBUG
#ifdef DOTRACE
#define log_trace() Log::Stream(Log::TRACE)
#else
#define log_trace() if(false) Log::Stream(Log::TRACE)
#endif
#define log_debug() Log::Stream(Log::DEBUG)
#else
#define log_trace() if(false) Log::Stream(Log::TRACE)
#define log_debug() if(false) Log::Stream(Log::DEBUG)
#endif

#define log_info()  Log::Stream(Log::INFO)
#define log_warn()  Log::Stream(Log::WARNING)
#define log_error() Log::Stream(Log::ERROR)
#define log_fatal() Log::Stream(Log::FATAL)

#endif
