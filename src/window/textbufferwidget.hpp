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

            class History {
                    typedef std::list<QString> linked_list_type;

                    static constexpr int MAX_SIZE = 1000;
                public:
                    using Iterator = linked_list_type::const_iterator;


                    History();


                    [[nodiscard]] inline Iterator begin() const {
                        return m_History.begin();
                    }

                    [[nodiscard]] inline Iterator end() const {
                        return m_History.end();
                    }

                    void push(const QString& newcmd);

                private:
                    linked_list_type m_History;
            };

        public:
            explicit TextBufferBrowser(TextBufferWidget* wParent);


            QVariant loadResource(int type, const QUrl& name) override;


            void pushInputStyle();


            [[nodiscard]] inline QTextBlockFormat inputBlockFormat() const {
                return m_InputBlockFormat;
            }

            inline void setInputBlockFormat(QTextBlockFormat f) {
                m_InputBlockFormat = std::move(f);
            }

            [[nodiscard]] inline QTextCharFormat inputCharFormat() const {
                return m_InputCharFormat;
            }

            inline void setInputCharFormat(QTextCharFormat f) {
                m_InputCharFormat = std::move(f);
            }

            [[nodiscard]] QString lineInputBuffer() const;

            void setLineInputBuffer(const QString& str);

            [[nodiscard]] inline int lineInputStartCursorPosition() const {
                return m_LineInputStartCursorPosition;
            }

            inline void setLineInputStartCursorPosition(int liscp) {
                m_LineInputStartCursorPosition = liscp;
            }

            [[nodiscard]] inline bool receivingLineInput() const {
                return m_LineInputStartCursorPosition >= 0;
            }

            inline void setImages(const std::vector<QImage>& imgs) {
                m_Images = imgs;
            }

        public slots:
            void onCursorPositionChanged();

        protected:
            void keyPressEvent(QKeyEvent* ev) override;

        private:
            QTextCharFormat m_InputCharFormat;
            QTextBlockFormat m_InputBlockFormat;

            std::vector<QImage> m_Images;

            int m_LineInputStartCursorPosition;

            History m_History;
            History::Iterator m_HistoryIterator;
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