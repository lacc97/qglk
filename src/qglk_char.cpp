#include <cstring>

#include <QChar>
#include <QLatin1Char>
#include <QString>
#include <QVector>

#include "glk.hpp"

unsigned char glk_char_to_lower(unsigned char ch) {
    QChar c((QLatin1Char(ch)));

    return static_cast<unsigned char>(c.toLower().toLatin1());
}

unsigned char glk_char_to_upper(unsigned char ch) {
    QChar c((QLatin1Char(ch)));

    return static_cast<unsigned char>(c.toUpper().toLatin1());
}

glui32 glk_buffer_to_lower_case_uni(glui32* buf, glui32 len, glui32 numchars) {
    for(glui32 i = 0; i < numchars; i++)
        buf[i] = QChar::toLower(buf[i]);

    return numchars;
}

glui32 glk_buffer_to_upper_case_uni(glui32* buf, glui32 len, glui32 numchars) {
    for(glui32 i = 0; i < numchars; i++)
        buf[i] = QChar::toUpper(buf[i]);

    return numchars;
}

glui32 glk_buffer_to_title_case_uni(glui32* buf, glui32 len, glui32 numchars, glui32 lowerrest) {
    if(numchars > 0)
        buf[0] = QChar::toUpper(buf[0]);
    else
        return numchars;

    if(lowerrest != 0) {
        for(glui32 i = 1; i < numchars; i++)
            buf[i] = QChar::toLower(buf[i]);
    }

    return numchars;
}

glui32 glk_buffer_canon_decompose_uni(glui32* buf, glui32 len, glui32 numchars) {
    QString in_str = QString::fromUcs4(buf, numchars);

    QVector<glui32> out_str = in_str.normalized(QString::NormalizationForm_KD).toUcs4();

    std::memcpy(buf, out_str.data(), std::min(static_cast<glui32>(out_str.size()), len)*sizeof(glui32));

    return static_cast<glui32>(out_str.size());
}

glui32 glk_buffer_canon_normalize_uni(glui32* buf, glui32 len, glui32 numchars) {
    QString in_str = QString::fromUcs4(buf, numchars);

    QVector<glui32> out_str = in_str.normalized(QString::NormalizationForm_KC).toUcs4();

    std::memcpy(buf, out_str.data(), std::min(static_cast<glui32>(out_str.size()), len)*sizeof(glui32));

    return static_cast<glui32>(out_str.size());
}

