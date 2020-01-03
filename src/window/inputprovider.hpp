#ifndef INPUTPROVIDER_HPP
#define INPUTPROVIDER_HPP

#include <memory>
#include <set>

#include <QObject>
#include <QPoint>
#include <QMutex>

#include "glk.hpp"

namespace Glk {
    class Window;

    class WindowController;

    class InputRequest : public QObject {
        Q_OBJECT

            Q_DISABLE_COPY(InputRequest)

        public:
            InputRequest();

            ~InputRequest() override = default;


            [[nodiscard]] virtual event_t generateEvent(Window* win) = 0;


            void cancel();


            [[nodiscard]] inline bool isCancelled() const {
                return m_Cancelled;
            }

            [[nodiscard]] inline bool isFulfilled() const {
                return m_Fulfilled;
            }

            [[nodiscard]] bool isPending() const;

        signals:

            void cancelled();

            void fulfilled();

        protected:
            inline void fulfill() {
                m_Fulfilled = true;
            }

            [[nodiscard]] inline QMutex* mutex() {
                return &m_Mutex;
            }

        private:
            QMutex m_Mutex;

            bool m_Fulfilled;
            bool m_Cancelled;
    };

    class CharInputRequest : public InputRequest {
        Q_OBJECT

            Q_DISABLE_COPY(CharInputRequest)

        public:
            explicit CharInputRequest(bool unicode);


            [[nodiscard]] event_t generateEvent(Window* win) override;


            [[nodiscard]] inline bool isUnicode() const {
                return m_Unicode;
            }

        public slots:

            void fulfill(Qt::Key key, const QString& ch);

        private:
            bool m_Unicode;

            glui32 m_Char;
    };

    class LineInputRequest : public InputRequest {
        Q_OBJECT

            Q_DISABLE_COPY(LineInputRequest)

        public:
            [[nodiscard]] static std::vector<Qt::Key> toQtKeyTerminators(glui32 keycode);

            LineInputRequest(void* buf, glui32 maxBufLen, glui32 initBufLen, bool unicode,
                             std::set<Qt::Key> terms, bool echoes);

            ~LineInputRequest() override;


            // also copies to buffer
            [[nodiscard]] event_t generateEvent(Window* win) override;


            template<typename CharT>
            [[nodiscard]] inline CharT* buffer() const {
                static_assert(std::is_integral_v<CharT> &&
                              (std::is_same_v<unsigned char, std::make_unsigned_t<CharT>> ||
                               std::is_same_v<glui32, CharT>));
                assert((isUnicode() ? std::is_same_v<glui32, CharT>
                                    : std::is_same_v<unsigned char, std::make_unsigned_t<CharT>>));

                return reinterpret_cast<CharT*>(m_Buffer);
            }

            [[nodiscard]] inline size_t bufferLength() const {
                return m_MaxBufferLength;
            }

            [[nodiscard]] inline bool isUnicode() const {
                return m_Unicode;
            }

            [[nodiscard]] inline bool lineEchoes() const {
                return m_Echoes;
            }

            [[nodiscard]] inline const std::set<Qt::Key>& lineTerminators() const {
                return m_Terminators;
            }

            [[nodiscard]] inline const QString& text() const {
                return m_Text;
            }

        public slots:

            void fulfill(Qt::Key terminator, const QString& text);

        private:
            bool m_Unicode;
            void* m_Buffer;
            size_t m_MaxBufferLength;
            std::set<Qt::Key> m_Terminators;
            bool m_Echoes;

            QString m_Text;
            Qt::Key m_Terminator;
    };

    class MouseInputRequest : public InputRequest {
        Q_OBJECT
        public:
            event_t generateEvent(Window* win) override;


            [[nodiscard]] inline const QPoint& point() const {
                return m_ClickPos;
            }

        public slots:

            void fulfill(const QPoint& qtpos);

        private:
            QPoint m_ClickPos;
    };

    class HyperlinkInputRequest : public InputRequest {
        Q_OBJECT
        public:
            event_t generateEvent(Window* win) override;


            [[nodiscard]] inline glui32 linkValue() const {
                return m_LinkValue;
            }

        public slots:

            void fulfill(glui32 linkval);

        private:
            glui32 m_LinkValue;
    };

    class InputProvider : public QObject {
        public:
            explicit InputProvider(WindowController* winController);

            [[nodiscard]] inline WindowController* controller() const {
                return mp_WindowsController;
            }

        private:
            WindowController* mp_WindowsController;
    };

    class KeyboardInputProvider : public InputProvider {
        Q_OBJECT
        public:
            explicit KeyboardInputProvider(WindowController* winController);

            [[nodiscard]] inline CharInputRequest* charInputRequest() const {
                return mp_CharInputRequest.get();
            }

            [[nodiscard]] inline bool lineInputEchoes() const {
                return m_LineInputEcho;
            }

            inline void setLineInputEcho(bool echoes) {
                m_LineInputEcho = echoes;
            }

            [[nodiscard]] inline LineInputRequest* lineInputRequest() const {
                return mp_LineInputRequest.get();
            }

            [[nodiscard]] inline const std::set<Qt::Key>& lineInputTerminators() const {
                return m_LineInputTerminators;
            }

            inline void setLineInputTerminators(const std::set<Qt::Key>& terminators) {
                m_LineInputTerminators = terminators;
            }

        public slots:

            void requestCharInput(bool unicode);

            void cancelCharInputRequest();

            void requestLineInput(void* buf, glui32 maxLen, glui32 initLen, bool unicode);

            void cancelLineInputRequest(event_t* ev);

        signals:

            void notifyCharInputRequested();

            void notifyCharInputRequestCancelled();

            void notifyCharInputRequestFulfilled();


            void notifyLineInputRequested();

            void notifyLineInputRequestCancelled(const QString& text, bool echoes);

            void notifyLineInputRequestFulfilled(const QString& text, bool echoes);

        protected slots:

            void onCharInputFulfilled();

            void onLineInputFulfilled();

        private:
            std::unique_ptr<CharInputRequest> mp_CharInputRequest;

            bool m_LineInputEcho;
            std::set<Qt::Key> m_LineInputTerminators;
            std::unique_ptr<LineInputRequest> mp_LineInputRequest;
    };

    class MouseInputProvider : public InputProvider {
        Q_OBJECT
        public:
            explicit MouseInputProvider(WindowController* winController);

            [[nodiscard]] inline MouseInputRequest* mouseInputRequest() const {
                return mp_MouseInputRequest.get();
            }

        public slots:

            void requestMouseInput();

            void cancelMouseInputRequest();

        signals:

            void notifyMouseInputRequested();

            void notifyMouseInputRequestCancelled();

            void notifyMouseInputRequestFulfilled(const QPoint& point);

        protected slots:

            void onMouseInputFulfilled();

        private:
            std::unique_ptr<MouseInputRequest> mp_MouseInputRequest;
    };

    class HyperlinkInputProvider : public InputProvider {
        Q_OBJECT
        public:
            explicit HyperlinkInputProvider(WindowController* winController);


            [[nodiscard]] inline HyperlinkInputRequest* hyperlinkInputRequest() const {
                return mp_HyperlinkInputRequest.get();
            }

        public slots:

            void requestHyperlinkInput();

            void cancelHyperlinkInputRequest();

        signals:

            void notifyHyperlinkInputRequested();

            void notifyHyperlinkInputRequestCancelled();

            void notifyHyperlinkInputRequestFulfilled(glui32 linkval);

        protected slots:

            void onHyperlinkInputFulfilled();

        private:
            std::unique_ptr<HyperlinkInputRequest> mp_HyperlinkInputRequest;
    };
}

#endif
