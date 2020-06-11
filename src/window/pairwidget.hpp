#ifndef QGLK_PAIRWIDGET_HPP
#define QGLK_PAIRWIDGET_HPP

#include <array>

#include <QWidget>

namespace Glk {
    class PairWidget : public QWidget {
            Q_OBJECT
        public:
            ~PairWidget();


            void clearChildren();

        protected:
            void childEvent(QChildEvent* event) override;

        private:
            std::array<QWidget*, 2> m_Children{};
    };

}

#endif //QGLK_PAIRWIDGET_HPP
