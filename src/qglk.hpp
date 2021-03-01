#ifndef QGLK_H
#define QGLK_H

#include <deque>
#include <list>
#include <map>
#include <functional>

#include <QCache>
#include <QMainWindow>
#include <QRunnable>
#include <QWidget>

#include <coroutine.h>

#include "dispatch.hpp"
#include "event/eventqueue.hpp"
#include "file/fileref.hpp"
#include "sound/schannel.hpp"
#include "thread/taskrequest.hpp"
#include "window/stylemanager.hpp"
#include "window/window.hpp"

namespace Ui {
    class QGlk;
}

namespace Glk {
    class Runnable : public QRunnable {
            Q_DISABLE_COPY(Runnable)
        public:
            Runnable(int argc_, char** argv_);

            void run() override;

            const QThread* glkThread() const {
                return mp_Thread;
            }

        private:
            QThread* mp_Thread;
            int argc;
            char** argv;
    };
}

class QGlk : public QMainWindow {
        Q_OBJECT
        Q_DISABLE_COPY(QGlk)

        friend int main(int, char**);
        friend void glk_tick();
//         friend void glk_select(event_t*);
    public:
        enum class GlkStatus {
            eEXITED,
            eINTERRUPTED
        };


        static QGlk& getMainWindow();

        ~QGlk();


        void addToDeleteQueue(Glk::WindowController* winController);

        QImage loadImage(glui32 image);

        void run();


        inline Glk::Window* rootWindow() const {
            return mp_RootWindow;
        }
        inline void setRootWindow(Glk::Window* win) {
            assert(!win || !win->parent());

            mp_RootWindow = win;
        }
        inline coroutine::Channel<GlkStatus>& statusChannel() const {
            return *mp_StatusChannel;
        }
        inline const Glk::Runnable* glkRunnable() const {
            return mp_Runnable;
        }
        inline const std::function<void(void)>& interruptHandler() {
            return m_InterruptHandler;
        }
        inline void setInterruptHandler(const std::function<void(void)>& handler) {
            m_InterruptHandler = handler;
        }
        inline const Glk::StyleManager& defaultStyleManager() const {
            return m_DefaultStyles;
        }
        inline Glk::StyleManager& textBufferStyleManager() {
            return m_TextBufferStyles;
        }

        inline Glk::Dispatch& dispatch() {
            return m_Dispatch;
        }
        inline Glk::EventQueue& eventQueue() {
            return m_EventQueue;
        }
        inline std::list<Glk::Window*>& windowList() {
            return m_WindowList;
        }
        inline std::list<strid_t>& streamList() {
            return m_StreamList;
        }
        inline std::list<frefid_t>& fileReferenceList() {
            return m_FileReferenceList;
        }
        inline std::list<Glk::SoundChannel*>& soundChannelList() {
            return m_SoundChannelList;
        }

        bool event(QEvent* event) override;

    public slots:
        void synchronize();

    signals:
        void tick();
        void poll();

    protected:
        void closeEvent(QCloseEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        QGlk(int argc, char** argv);

        bool handleGlkTask(Glk::TaskEvent* event);

        Ui::QGlk* mp_UI;
        std::unique_ptr<coroutine::Channel<GlkStatus>> mp_StatusChannel;
        Glk::Runnable* mp_Runnable;
        Glk::Window* mp_RootWindow;
        std::deque<Glk::WindowController*> m_DeleteQueue;
        Glk::EventQueue m_EventQueue;
        std::list<Glk::Window*> m_WindowList;
        std::list<strid_t> m_StreamList;
        std::list<frefid_t> m_FileReferenceList;
        std::list<Glk::SoundChannel*> m_SoundChannelList;

        std::function<void(void)> m_InterruptHandler;

        QCache<glui32, QImage> m_ImageCache;
        Glk::StyleManager m_DefaultStyles;
        Glk::StyleManager m_TextBufferStyles;

        Glk::Dispatch m_Dispatch;
};

#endif // QGLK_H
