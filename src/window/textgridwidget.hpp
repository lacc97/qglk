#ifndef TEXTGRIDWIDGET_HPP
#define TEXTGRIDWIDGET_HPP

#include <vector>

#include <QWidget>

#include "glk.hpp"

namespace Glk {
    class TextGridWidget : public QWidget {
        Q_OBJECT
        public:
            TextGridWidget();

            inline void setGrid(const std::vector<std::vector<glui32>> newGrid) {
                m_Grid = newGrid;
            }

//            [[nodiscard]] inline int horizontalMargins() const {
//                return contentsMargins().left() + contentsMargins().right();
//            }
//
//            [[nodiscard]] inline int verticalMargins() const {
//                return contentsMargins().top() + contentsMargins().bottom();
//            }

        signals:

            void resized();

        protected:
            void paintEvent(QPaintEvent* event) override;

            void resizeEvent(QResizeEvent* event) override;

        private:
            std::vector<std::vector<glui32>> m_Grid;
    };
}


#endif //TEXTGRIDWIDGET_HPP
