#include "style.hpp"

#include <QDebug>
#include <QFontDatabase>
#include <QTextBrowser>

Glk::Style::Style(Glk::Style::Type type_) : m_Type(type_), m_Font(QFontDatabase::systemFont(QFontDatabase::GeneralFont)), m_Indentation(0), m_ParaIndentation(0), m_Justification(stylehint_just_LeftRight), m_FontSizeIncrease(0), m_TextColor(Qt::black), m_BackgroundColor(Qt::white) {
    switch(m_Type) { // TODO load these up from a stylesheet file?
        case Emphasized:
            m_Font.setWeight(QFont::Bold);
            break;

        case Preformatted:
            m_Font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            break;

        case Header:
            m_Font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
            m_Font.setWeight(QFont::Bold);
            m_Font.setCapitalization(QFont::SmallCaps);
            m_Justification = stylehint_just_Centered;
            break;

        case Subheader:
            m_Font = QFontDatabase::systemFont(QFontDatabase::TitleFont);
            m_Font.setWeight(QFont::Bold);
            break;

        case Alert:
            m_Font.setItalic(true);
            m_Font.setWeight(QFont::Bold);
            break;

        case Note:
            m_Font.setItalic(true);
            break;

        case BlockQuote:
            m_Font.setItalic(true);
            m_Justification = stylehint_just_Centered;
            break;

        case Input:
            m_Font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
            break;

        default: // Normal, User1, User2
            break;
    }
}

Qt::AlignmentFlag toAlignmentFlag(glui32 just) {
    switch(just) {
        case stylehint_just_Centered:
            return Qt::AlignHCenter;

        case stylehint_just_LeftRight:
            return Qt::AlignJustify;

        case stylehint_just_RightFlush:
            return Qt::AlignRight;

        case stylehint_just_LeftFlush:
        default:
            return Qt::AlignLeft;
    }
}

const QTextBlockFormat Glk::Style::blockFormat() const {
    QTextBlockFormat blk;

    blk.setAlignment(toAlignmentFlag(m_Justification));
    blk.setIndent(0.1 * m_Indentation);
    blk.setTextIndent(0.1 * m_ParaIndentation);
    blk.setBottomMargin(blk.bottomMargin()*1.5);

    return blk;
}

const QTextCharFormat Glk::Style::charFormat() const {
    QTextCharFormat ch;

    ch.setFont(m_Font);
    ch.setForeground(m_TextColor);

    return ch;
}

glui32 Glk::Style::getHint(glui32 hint) const {
    switch(hint) {
        case stylehint_Indentation:
            return *reinterpret_cast<const glui32*>(&m_Indentation);

        case stylehint_ParaIndentation:
            return *reinterpret_cast<const glui32*>(&m_ParaIndentation);

        case stylehint_Justification:
            return m_Justification;

        case stylehint_Size:
            return m_FontSizeIncrease;

        case stylehint_Weight:
            switch(m_Font.weight()) {
                case QFont::Bold:
                    return 1;

                case QFont::Light:
                    return -1;

                case QFont::Normal:
                default:
                    return 0;
            }

        case stylehint_Oblique:
            return (m_Font.italic() ? TRUE : FALSE);

        case stylehint_Proportional:
            return (m_Font.fixedPitch() ? TRUE : FALSE);

        case stylehint_TextColor:
            return m_TextColor.rgb();

        case stylehint_BackColor:
            return m_BackgroundColor.rgb();
    }

    return 0;
}

bool Glk::Style::measureHint(glui32 hint, glui32* result) const {
    Q_ASSERT(result);

    switch(hint) {
        case stylehint_Indentation:
            *result = *reinterpret_cast<const glui32*>(&m_Indentation);
            return true;

        case stylehint_ParaIndentation:
            *result = *reinterpret_cast<const glui32*>(&m_ParaIndentation);
            return true;

        case stylehint_Justification:
            *result = m_Justification;
            return true;

        case stylehint_Size:
            *result = m_Font.pointSize();
            return true;

        case stylehint_Weight:
            switch(m_Font.weight()) {
                case QFont::Bold:
                    *result = 1;
                    break;

                case QFont::Light:
                    *result = -1;
                    break;

                case QFont::Normal:
                    *result = 0;
                    break;

                default:
                    return false;
            }

            return true;

        case stylehint_Oblique:
            *result = (m_Font.italic() ? 1 : 0);
            return true;

        case stylehint_Proportional:
            *result = (m_Font.fixedPitch() ? 1 : 0);
            return true;

        case stylehint_TextColor:
            *result = m_TextColor.rgb();
            return true;

        case stylehint_BackColor:
            *result = m_BackgroundColor.rgb();
            return false;
    }

    return false;
}

void Glk::Style::setHint(glui32 hint, glui32 value) {
    switch(hint) {
        case stylehint_Indentation:
            m_Indentation = *reinterpret_cast<glsi32*>(&value);
            break;

        case stylehint_ParaIndentation:
            m_ParaIndentation = *reinterpret_cast<glsi32*>(&value);
            break;

        case stylehint_Justification:
            m_Justification = value;
            break;

        case stylehint_Size:
            m_Font.setPointSize(m_Font.pointSize() - m_FontSizeIncrease);
            m_FontSizeIncrease = value;
            m_Font.setPointSize(m_Font.pointSize() + m_FontSizeIncrease);
            break;

        case stylehint_Weight:
            switch(*reinterpret_cast<glsi32*>(&value)) {
                case 1:
                    m_Font.setWeight(QFont::Bold);
                    break;

                case -1:
                    m_Font.setWeight(QFont::Light);
                    break;

                case 0:
                default:
                    m_Font.setWeight(QFont::Normal);
                    break;
            }

        case stylehint_Oblique:
            m_Font.setItalic(value);
            break;

        case stylehint_Proportional:
            m_Font.setFixedPitch(value);
            break;

        case stylehint_TextColor:
            m_TextColor.setRgb(value);
            break;

        case stylehint_BackColor:
            m_TextColor.setRgb(value);
            break;
    }
}

bool Glk::Style::operator==(const Glk::Style& other) const {
    if(m_Indentation != other.m_Indentation)
        return true;

    if(m_ParaIndentation != other.m_ParaIndentation)
        return true;

    if(m_Justification)
        return true;

    if(m_Font != other.m_Font)
        return true;

    if(m_TextColor == other.m_TextColor)
        return true;

    if(m_BackgroundColor == other.m_BackgroundColor)
        return true;

    return false;
}
