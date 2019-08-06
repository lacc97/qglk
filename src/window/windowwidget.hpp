#ifndef WINDOWWIDGET_HPP
#define WINDOWWIDGET_HPP


#include <set>

#include <QWidget>

#include "glk.hpp"

namespace Glk {
    class WindowWidget : public QWidget {
        Q_OBJECT
        public:
            WindowWidget();

            ~WindowWidget() override = default;


            [[nodiscard]] bool eventFilter(QObject* obj, QEvent* ev) override;


            [[nodiscard]] inline bool charInputPending() const {
                return m_ReceivingCharInput;
            }

            [[nodiscard]] inline bool hyperlinkInputPending() const {
                return m_ReceivingHyperlinkInput;
            }

            [[nodiscard]] inline bool lineInputPending() const {
                return m_ReceivingLineInput;
            }

            [[nodiscard]] inline bool mouseInputPending() const {
                return m_ReceivingMouseInput;
            }

        public slots:
            void cancelCharInput();

            void cancelHyperlinkInput();

            void cancelLineInput();

            void cancelMouseInput();

            void requestCharInput();

            void requestHyperlinkInput();

            void requestLineInput(glui32 maxInputLength, const std::set<Qt::Key>& terminators);

            void requestMouseInput();

        signals:
            void characterInput(Qt::Key key, const QString& text);

            void hyperlinkInput(glui32 linkVal);

            void lineInput(Qt::Key terminator, const QString& text);

            void mouseInput(const QPoint& pos);

        protected:
            void installInputFilter(QWidget* widget);


            [[nodiscard]] virtual QString lineInputBuffer();

            virtual void onCharInputRequested();

            virtual void onCharInputFinished();

            virtual void onHyperlinkInputRequested();

            virtual void onLineInputRequested();

            virtual void onLineInputFinished();

            virtual void onMouseInputRequested();

            virtual void onMouseInputFinished();

        private:
            [[nodiscard]] bool handleKeyPressEvent(QKeyEvent* ev);

            [[nodiscard]] bool handleMousePressEvent(QMouseEvent* ev);

            [[nodiscard]] bool isLineInputTerminatorKey(Qt::Key key) {
                return (key == Qt::Key_Return || key == Qt::Key_Enter ||
                        m_LineTerminators.find(static_cast<Qt::Key>(key)) != m_LineTerminators.end());
            }


            QWidget* mp_InputWidget;

            bool m_ReceivingCharInput;

            bool m_ReceivingHyperlinkInput;

            bool m_ReceivingLineInput;
            std::set<Qt::Key> m_LineTerminators;

            bool m_ReceivingMouseInput;
    };
}


#endif //WINDOWWIDGET_HPP
