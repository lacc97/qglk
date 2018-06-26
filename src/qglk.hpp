#ifndef QGLK_H
#define QGLK_H

#include <functional>

#include <QLinkedList>
#include <QMainWindow>
#include <QRunnable>
#include <QWidget>

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
        public:
            Runnable(int argc_, char** argv_);
            
            void run() override;
            
    private:
        int argc;
        char** argv;
    };
}

class QGlk : public QMainWindow {
        Q_OBJECT

        friend int main(int, char**);
        friend void glk_tick();
    public:
        static QGlk& getMainWindow();

        ~QGlk();
        
        void run();
        
        inline Glk::Window* rootWindow() const {
            return mp_RootWindow;
        }
        inline void setRootWindow(Glk::Window* win) {
            if(win)
                win->setWindowParent(NULL);
            mp_RootWindow = win;
            setCentralWidget(mp_RootWindow ? mp_RootWindow : new QWidget(this));
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

        inline Glk::EventQueue& eventQueue() {
            return m_EventQueue;
        }
        inline QLinkedList<Glk::Window*>& windowList() {
            return m_WindowList;
        }
        inline QLinkedList<Glk::Stream*>& streamList() {
            return m_StreamList;
        }
        inline QLinkedList<Glk::FileReference*>& fileReferenceList() {
            return m_FileReferenceList;
        }
        inline QLinkedList<Glk::SoundChannel*>& soundChannelList() {
            return m_SoundChannelList;
        }

        bool event(QEvent* event) override;

    protected:
        void closeEvent(QCloseEvent * event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        QGlk(int argc, char** argv);

        bool handleGlkTask(Glk::TaskEvent* event);

        Ui::QGlk* ui;
        Glk::Runnable* mp_Runnable;
        Glk::Window* mp_RootWindow;
        Glk::EventQueue m_EventQueue;
        QLinkedList<Glk::Window*> m_WindowList;
        QLinkedList<Glk::Stream*> m_StreamList;
        QLinkedList<Glk::FileReference*> m_FileReferenceList;
        QLinkedList<Glk::SoundChannel*> m_SoundChannelList;
        
        std::function<void(void)> m_InterruptHandler;
        
        Glk::StyleManager m_DefaultStyles;
        Glk::StyleManager m_TextBufferStyles;
};

#endif // QGLK_H
