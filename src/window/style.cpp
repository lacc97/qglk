#include "style.hpp"

#include <QDebug>
#include <QFontDatabase>

Glk::Style::Style(Glk::Style::Type type_) : m_Type(type_), m_Font(QFontDatabase::systemFont(QFontDatabase::GeneralFont)), m_Indentation(0), m_ParaIndentation(0), m_Justification(stylehint_just_LeftFlush), m_FontSizeIncrease(0) {
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

QString toJustificationString(glui32 just) {
    switch(just) {
        case stylehint_just_Centered:
            return "center";

        case stylehint_just_LeftRight:
            return "justify";

        case stylehint_just_RightFlush:
            return "right";

        case stylehint_just_LeftFlush:
        default:
            return "left";
    }
}

QString Glk::Style::styleString() const {
    QString fontFamilyString = QStringLiteral("font-family: '%1', '%2'").arg(m_Font.family()).arg(m_Font.lastResortFamily());
    QString fontStyleString = QStringLiteral("font-style: %1").arg(m_Font.italic() ? "italic" : "normal");
    QString fontSizeString = QStringLiteral("font-size: %1pt").arg(m_Font.pointSize());
    QString fontWeightString = QStringLiteral("font-weight: %1").arg(m_Font.bold() ? "bold" : "normal");
    QString fontVariantString = QStringLiteral("font-variant: %1").arg(m_Font.capitalization() == QFont::SmallCaps ? "small-caps" : "normal");
    QString fontString = QStringLiteral("%1; %2; %3; %4;%5").arg(fontFamilyString).arg(fontStyleString).arg(fontSizeString).arg(fontWeightString).arg(fontVariantString);

    QString justificationString = QStringLiteral("text-align: %1").arg(toJustificationString(m_Justification));
    
    QString paraIndentationString = QStringLiteral("text-indent: %1px").arg(5*m_ParaIndentation);
    
    // TODO regular indentation
    
    return QStringLiteral("%1; %2; %3").arg(fontString).arg(justificationString).arg(paraIndentationString);
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

    return false;
}
