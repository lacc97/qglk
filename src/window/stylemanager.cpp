#include "stylemanager.hpp"

#define SETUP_STYLE(n) m_StyleMap[Style::n] = Style(Style::n)

Glk::StyleManager::StyleManager() : m_StyleMap() {
    SETUP_STYLE(Normal);
    SETUP_STYLE(Emphasized);
    SETUP_STYLE(Preformatted);
    SETUP_STYLE(Header);
    SETUP_STYLE(Subheader);
    SETUP_STYLE(Alert);
    SETUP_STYLE(Note);
    SETUP_STYLE(BlockQuote);
    SETUP_STYLE(Input);
    SETUP_STYLE(User1);
    SETUP_STYLE(User2);
}

Glk::Style& Glk::StyleManager::operator[](Style::Type type) {
    return m_StyleMap[type];
}

const Glk::Style& Glk::StyleManager::operator[](Style::Type type) const {
    if(m_StyleMap.contains(type))
        return *m_StyleMap.find(type);
    else
        return *m_StyleMap.find(Style::Normal);
}
