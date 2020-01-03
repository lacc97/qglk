#ifndef TEXTBUFFERWIDGET_HPP
#define TEXTBUFFERWIDGET_HPP


#include <set>

#include <QImage>
#include <QTextBrowser>

#include "windowwidget.hpp"

namespace Glk {
    class TextBufferWidget;

    class TextBufferBrowser : public QTextBrowser {
        Q_OBJECT
        public:
            explicit TextBufferBrowser(TextBufferWidget* wParent);


            QVariant loadResource(int type, const QUrl& name) override;


            [[nodiscard]] inline int lineInputStartCursorPosition() const {
                return m_LineInputStartCursorPosition;
            }

            inline void setLineInputStartCursorPosition(int liscp) {
                m_LineInputStartCursorPosition = liscp;
            }

            inline void setImages(const std::vector<QImage>& imgs) {
                m_Images = imgs;
            }

        public slots:
            void onCursorPositionChanged();

        protected:
            void keyPressEvent(QKeyEvent* ev) override;

        private:
            std::vector<QImage> m_Images;
            int m_LineInputStartCursorPosition;

    };

    class TextBufferWidget : public WindowWidget {
            Q_OBJECT
        public:
            TextBufferWidget();


            [[nodiscard]] inline TextBufferBrowser* browser() const {
                return mp_Browser;
            }

        protected:
            QString lineInputBuffer() override;

            void onLineInputRequested() override;

            void onLineInputFinished() override;

        protected slots:
            void onHyperlinkPressed(const QUrl& url);

        private:
            TextBufferBrowser* mp_Browser;
    };
}


#endif //TEXTBUFFERWIDGET_HPP

//#ifndef TEXTBUFFERWIDGET_HPP
//#define TEXTBUFFERWIDGET_HPP
//
//
//#include <set>
//
//#include <QImage>
//#include <QTextBrowser>
//#include <QWidget>
//
//#include "glk.hpp"
//
//namespace Glk {
//    class TextBufferWidget;
//
//    class TextBufferBrowser : public QTextBrowser {
//        Q_OBJECT
//        public:
//            explicit TextBufferBrowser(TextBufferWidget* wParent);
//
//
//            QVariant loadResource(int type, const QUrl& name) override;
//
//
//            [[nodiscard]] inline bool charInputPending() const {
//                return m_ReceivingCharInput;
//            }
//
//            [[nodiscard]] inline bool lineInputPending() const {
//                return m_ReceivingLineInput;
//            }
//
//            void requestCharInput();
//
//            void requestLineInput();
//
//
//            inline void setImages(const std::vector<QImage>& imgs) {
//                m_Images = imgs;
//            }
//
//            inline void setLineTerminators(const std::set<Qt::Key>& terminators) {
//                m_LineTerminators = terminators;
//            }
//
//        public slots:
//
//            void onCharInputRequestCancelled();
//
//            void onCursorPositionChanged();
//
//            void onLineInputRequestCancelled();
//
//        signals:
//
//            void characterInput(Qt::Key key, const QString& text);
//
//            void lineInput(Qt::Key terminator, const QString& text);
//
//        protected:
//            void endCharInputRequest();
//
//            void endLineInputRequest(Qt::Key terminator);
//
//            void keyPressEvent(QKeyEvent* ev) override;
//
//        private:
//            std::vector<QImage> m_Images;
//            bool m_ReceivingCharInput;
//            bool m_ReceivingLineInput;
//            std::set<Qt::Key> m_LineTerminators;
//            int m_LineInputStartCursorPosition;
//
//    };
//
//    class TextBufferWidget : public QWidget {
//        public:
//            TextBufferWidget();
//
//
//            [[nodiscard]] inline TextBufferBrowser* browser() const {
//                return mp_Browser;
//            }
//
//        private:
//            TextBufferBrowser* mp_Browser;
//    };
//}
//
//
//#endif //TEXTBUFFERWIDGET_HPP
//
//#ifndef TEXTBUFFERWIDGET_HPP
//#define TEXTBUFFERWIDGET_HPP
//
//
//#include <set>
//
//#include <QImage>
//#include <QTextBrowser>
//#include <QWidget>
//
//#include "glk.hpp"
//
//namespace Glk {
//    class TextBufferWidget;
//
//    class TextBufferBrowser : public QTextBrowser {
//        Q_OBJECT
//        public:
//            explicit TextBufferBrowser(TextBufferWidget* wParent);
//
//
//            QVariant loadResource(int type, const QUrl& name) override;
//
//
//            [[nodiscard]] inline bool charInputPending() const {
//                return m_ReceivingCharInput;
//            }
//
//            [[nodiscard]] inline bool lineInputPending() const {
//                return m_ReceivingLineInput;
//            }
//
//            void requestCharInput();
//
//            void requestLineInput();
//
//
//            inline void setImages(const std::vector<QImage>& imgs) {
//                m_Images = imgs;
//            }
//
//            inline void setLineTerminators(const std::set<Qt::Key>& terminators) {
//                m_LineTerminators = terminators;
//            }
//
//        public slots:
//
//            void onCharInputRequestCancelled();
//
//            void onCursorPositionChanged();
//
//            void onLineInputRequestCancelled();
//
//        signals:
//
//            void characterInput(Qt::Key key, const QString& text);
//
//            void lineInput(Qt::Key terminator, const QString& text);
//
//        protected:
//            void endCharInputRequest();
//
//            void endLineInputRequest(Qt::Key terminator);
//
//            void keyPressEvent(QKeyEvent* ev) override;
//
//        private:
//            std::vector<QImage> m_Images;
//            bool m_ReceivingCharInput;
//            bool m_ReceivingLineInput;
//            std::set<Qt::Key> m_LineTerminators;
//            int m_LineInputStartCursorPosition;
//
//    };
//
//    class TextBufferWidget : public QWidget {
//        public:
//            TextBufferWidget();
//
//
//            [[nodiscard]] inline TextBufferBrowser* browser() const {
//                return mp_Browser;
//            }
//
//        private:
//            TextBufferBrowser* mp_Browser;
//    };
//}
//
//
//#endif //TEXTBUFFERWIDGET_HPP
