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

    dt.addYears(gds.year - 1970);
    dt.addMonths(gds.month - 1);
    dt.addDays(gds.day);
    dt.addSecs(gds.hour * 3600 + gds.minute * 60 + gds.second);
    dt.addMSecs(gds.microsec / 1000);

    return dt;
}

void glk_current_time(glktimeval_t* time) {
    QDateTime now = QDateTime::currentDateTime();
    *reinterpret_cast<qint64*>(&(time->high_sec)) = now.toSecsSinceEpoch();
    time->microsec = glsi32(now.toMSecsSinceEpoch() % 1000) * 1000;
}

glsi32 glk_current_simple_time(glui32 factor) {
    return glsi32(QDateTime::currentSecsSinceEpoch() / factor);
}

void glk_time_to_date_utc(glktimeval_t* time, glkdate_t* date) {
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(*reinterpret_cast<qint64*>(&(time->high_sec)) + (qint32(time->microsec) / 1000)).toUTC();
    fromQDateTime(dt, *date);
}

void glk_time_to_date_local(glktimeval_t* time, glkdate_t* date) {
    QDateTime dt = QDateTime::fromMSecsSinceEpoch(*reinterpret_cast<qint64*>(&(time->high_sec)) + (qint32(time->microsec) / 1000)).toLocalTime();

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
    *reinterpret_cast<qint64*>(&(time->high_sec)) = dt.toSecsSinceEpoch();
    time->microsec = glsi32(dt.toMSecsSinceEpoch() % 1000) * 1000;
}

void glk_date_to_time_local(glkdate_t* date, glktimeval_t* time) {
    QDateTime dt = QDateTime::currentDateTime();
    toQDateTime(*date, dt);
    *reinterpret_cast<qint64*>(&(time->high_sec)) = dt.toSecsSinceEpoch();
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
