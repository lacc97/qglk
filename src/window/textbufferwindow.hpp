#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

#include <QLinkedList>
#include <QTextBrowser>

#include "stylemanager.hpp"
#include "window.hpp"

namespace Glk {
    class TextBufferWindow;

    class TextBufferDevice : public QIODevice {
            Q_OBJECT

            // TODO all of this seems unnecessary
            class Block {
                public:
                    Block() : m_Words(), m_IsHyperlinkTagOpen(false) {}

                    // Plain text
                    void appendWords(QStringList words, const QString& styleString, const QString& styleStringNoColour);
                    void insertOpenHyperlinkTag(glui32 linkval);
                    void insertCloseHyperlinkTag();

                    void writeToBrowser(QTextBrowser* qtb) const;

                private:
                    QStringList m_Words;
                    bool m_IsHyperlinkTagOpen;
            };

        public:
            TextBufferDevice(TextBufferWindow* win);

            qint64 readData(char* data, qint64 maxlen) override;
            qint64 writeData(const char* data, qint64 len) override;

        public slots:
            void discard();
            void flush();
            void onHyperlinkPushed(glui32 linkval);
            void onWindowStyleChanged(const QString& newStyleString, const QString& newStyleStringNoColour);

        signals:
            void textChanged();

        private:
            TextBufferWindow* mp_TBWindow;
            QString m_StyleString;
            QString m_StyleStringNoColour;
            QList<Block> m_Buffer;
            glui32 m_CurrentHyperlink;
    };

    class TextBufferWindow : public Window {
            Q_OBJECT

            friend class TextBufferDevice;

            class History {
                    typedef QLinkedList<QString> linked_list_type;

                    static constexpr int MAX_SIZE = 1000;
                public:
                    History();
                    
                    void push(const QString& newcmd);

                    void resetIterator();
                    const QString next();
                    const QString previous();

                private:
                    linked_list_type m_History;
                    linked_list_type::iterator m_Iterator;
            };

        public:
            TextBufferWindow(glui32 rock_ = 0);
            ~TextBufferWindow() {}

            inline const Glk::StyleManager& styles() const {
                return m_Styles;
            }

            inline Glk::Style::Type getStyle() const {
                return m_CurrentStyleType;
            }
            void setStyle(Glk::Style::Type style) override;

            Glk::Window::Type windowType() const override {
                return Window::TextBuffer;
            }

            void clearWindow() override;

        public slots:
            void onHyperlinkClicked(const QUrl& link);
            void onTextChanged();

        signals:
            void styleChanged(const QString& newStyleString);

        protected:
            inline Glk::TextBufferDevice* ioDevice() const {
                return static_cast<Glk::TextBufferDevice*>(windowStream()->getIODevice());
            }

            void resizeEvent(QResizeEvent* ev) override;

            QSize pixelsToUnits(const QSize& pixels) const override;
            QSize unitsToPixels(const QSize& units) const override;

        protected slots:
            void onCharacterInputRequested();
            void onCharacterInputRequestEnded(bool cancelled);
            void onLineInputRequested();
            void onLineInputRequestEnded(bool cancelled, void* buf, glui32 len, bool unicode);

            void onCharacterInput(glui32 ch, bool doUpdate = true);
            void onSpecialCharacterInput(glui32 ch, bool doUpdate = true);

        private:
            QTextBrowser* mp_Text;
            History m_History;
            Glk::StyleManager m_Styles;
            Glk::Style::Type m_CurrentStyleType;
            Glk::Style::Type m_PreviousStyleType;
    };
}

#endif
