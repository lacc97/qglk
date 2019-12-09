#include <cstring>

#include <QDateTime>

#include "glk.hpp"

inline void fromQDateTime(const QDateTime& dt, glkdate_t& gds) {
    gds.year = dt.date().year();
    gds.month = dt.date().month();
    gds.day = dt.date().day();
    gds.weekday = dt.date().dayOfWeek() % 7;
    gds.hour = dt.time().hour();
    gds.minute = dt.time().minute();
    gds.second = dt.time().second();
    gds.microsec = dt.time().msec() * 1000;
}

inline QDateTime toQDateTime(const glkdate_t& gds, QDateTime& dt) {
    dt.setSecsSinceEpoch(0);

    dt = dt.addYears(gds.year - 1970);
    dt = dt.addMonths(gds.month - 1);
    dt = dt.addDays(gds.day);
    dt = dt.addSecs(gds.hour * 3600 + gds.minute * 60 + gds.second);
    dt = dt.addMSecs(gds.microsec / 1000);

    return dt;
}

void glk_current_time(glktimeval_t* time) {
    QDateTime now = QDateTime::currentDateTime();

    qint64 secs = now.toSecsSinceEpoch();
    {
        quint64 secsBits;
        std::memcpy(&secsBits, &secs, sizeof(secsBits));
        time->low_sec = (secsBits & 0xffffffff);

        quint32 highSecsBits = (secsBits >> 32u);
        std::memcpy(&time->high_sec, &highSecsBits, sizeof(time->high_sec));
    }

    time->microsec = glsi32(now.toMSecsSinceEpoch() % 1000) * 1000;
}

glsi32 glk_current_simple_time(glui32 factor) {
    return glsi32(QDateTime::currentSecsSinceEpoch() / factor);
}

void glk_time_to_date_utc(glktimeval_t* time, glkdate_t* date) {
    qint64 secs = (qint64(time->high_sec) << 32ul) | qint64(time->low_sec);
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(secs + (qint32(time->microsec) / 1000)).toUTC();
    fromQDateTime(dt, *date);
}

void glk_time_to_date_local(glktimeval_t* time, glkdate_t* date) {
    qint64 secs = (qint64(time->high_sec) << 32ul) | qint64(time->low_sec);
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(secs + (qint32(time->microsec) / 1000)).toLocalTime();

    fromQDateTime(dt, *date);
}

void glk_simple_time_to_date_utc(glsi32 time, glui32 factor, glkdate_t* date) {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(time * factor).toUTC();
    fromQDateTime(dt, *date);
}

void glk_simple_time_to_date_local(glsi32 time, glui32 factor, glkdate_t* date) {
    QDateTime dt = QDateTime::fromSecsSinceEpoch(time * factor).toLocalTime();
    fromQDateTime(dt, *date);
}

void glk_date_to_time_utc(glkdate_t* date, glktimeval_t* time) {
    QDateTime dt = QDateTime::currentDateTimeUtc();
    toQDateTime(*date, dt);

    qint64 secs = dt.toSecsSinceEpoch();
    {
        quint64 secsBits;
        std::memcpy(&secsBits, &secs, sizeof(secsBits));
        time->low_sec = (secsBits & 0xffffffff);

        quint32 highSecsBits = (secsBits >> 32u);
        std::memcpy(&time->high_sec, &highSecsBits, sizeof(time->high_sec));
    }

    time->microsec = glsi32(dt.toMSecsSinceEpoch() % 1000) * 1000;
}

void glk_date_to_time_local(glkdate_t* date, glktimeval_t* time) {
    QDateTime dt = QDateTime::currentDateTime();
    toQDateTime(*date, dt);

    qint64 secs = dt.toSecsSinceEpoch();
    {
        quint64 secsBits;
        std::memcpy(&secsBits, &secs, sizeof(secsBits));
        time->low_sec = (secsBits & 0xffffffff);

        quint32 highSecsBits = (secsBits >> 32u);
        std::memcpy(&time->high_sec, &highSecsBits, sizeof(time->high_sec));
    }

    time->microsec = glsi32(dt.toMSecsSinceEpoch() % 1000) * 1000;
}

glsi32 glk_date_to_simple_time_utc(glkdate_t* date, glui32 factor) {
    QDateTime dt = QDateTime::currentDateTimeUtc();
    toQDateTime(*date, dt);
    return glsi32(dt.toSecsSinceEpoch() / factor);
}

glsi32 glk_date_to_simple_time_local(glkdate_t* date, glui32 factor) {
    QDateTime dt = QDateTime::currentDateTime();
    toQDateTime(*date, dt);
    return glsi32(dt.toSecsSinceEpoch() / factor);
}
