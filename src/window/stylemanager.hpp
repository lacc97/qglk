#ifndef STYLEMANAGER_HPP
#define STYLEMANAGER_HPP

#include <QHash>

#include "style.hpp"

namespace Glk {
    class StyleManager {
        public:
            StyleManager();
            StyleManager(const StyleManager&) = default;
            StyleManager(StyleManager&&) = default;

            StyleManager& operator=(const StyleManager&) = default;
            StyleManager& operator=(StyleManager &&) = default;

            const Style& operator[](Style::Type type) const;
            Style& operator[](Style::Type type);

        private:
            QHash<Style::Type, Style> m_StyleMap;
    };
}

#endif
