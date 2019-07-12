#ifndef TEXTBUFFERWINDOW_HPP
#define TEXTBUFFERWINDOW_HPP

#include <memory>
#include <vector>

#include <QLinkedList>
#include <QTextBrowser>
#include <QVector>

#include "stylemanager.hpp"
#include "window.hpp"

namespace Glk {
    class TextBufferWindow;

    class TextBufferDevice : public QIODevice {
            Q_OBJECT

            class Command {
                public:
                    virtual ~Command() = default;

                    virtual void perform(QTextCursor& cursor) const = 0;
            };

            class WriteCommand : public Command {
                public:
                    WriteCommand(const QString& qstr, const QTextBlockFormat& blkfmt, const QTextCharFormat& chfmt);

                    void perform(QTextCursor& cursor) const override;

                    inline void appendText(const QString& qstr) {
                        m_Text.append(qstr);
                    }

                private:
                    QString m_Text;
                    QTextBlockFormat m_BlockFormat;
                    QTextCharFormat m_CharFormat;
            };

            class ImageCommand : public Command {
                public:
                    ImageCommand(int imgnum, glsi32 alignment, glui32 w, glui32 h);

                    void perform(QTextCursor& cursor) const override;

                private:
                    int m_ImageNumber;
                    glsi32 m_Alignment;
                    glui32 m_Width;
                    glui32 m_Height;
            };

            class ClearCommand : public Command {
                public:
                    void perform(QTextCursor& cursor) const override;
            };

        public:
            TextBufferDevice(TextBufferWindow* win);

            void drawImage(int imgnum, glsi32 alignment, glui32 w, glui32 h);

            qint64 readData(char* data, qint64 maxlen) override;
            qint64 writeData(const char* data, qint64 len) override;

        public slots:
            void clear();
            void discard();
            void flush();
            void onHyperlinkPushed(glui32 linkval);
            void onWindowStyleChanged(const QTextBlockFormat& blkfmt, const QTextCharFormat& chfmt);

        signals:
            void textChanged();

        private:
            TextBufferWindow* mp_TBWindow;
            QTextBlockFormat m_CurrentBlockFormat;
            QTextCharFormat m_CurrentNonHLCharFormat;
            QTextCharFormat m_CurrentCharFormat;
            std::vector<std::unique_ptr<Command>> m_Buffer;
            glui32 m_CurrentHyperlink;
    };

    class TextBufferBrowser : public QTextBrowser {
        public:
            TextBufferBrowser(QWidget* parent = NULL) : QTextBrowser(parent) {}

            int numImages() const;

            void addImage(const QImage& im);
            void clearImages();

            QVariant loadResource(int type, const QUrl& name) override;

        protected:
            void keyPressEvent(QKeyEvent* event) override;

        private:
            QList<QImage> m_ImageList;
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

            bool drawImage(const QImage& im, glsi32 alignment, glui32 w, glui32 h);

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
            TextBufferBrowser* mp_Text;
            History m_History;
            Glk::StyleManager m_Styles;
            Glk::Style::Type m_CurrentStyleType;
            Glk::Style::Type m_PreviousStyleType;
    };
}

#endif
