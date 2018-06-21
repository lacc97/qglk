#ifndef QGLK_H
#define QGLK_H

#include <functional>

#include <QMainWindow>
#include <QRunnable>
#include <QWidget>

#include "event/eventqueue.hpp"
#include "thread/taskrequest.hpp"
#include "window/stylemanager.hpp"
#include "window/window.hpp"

namespace Ui {
    class QGlk;
}

namespace Glk {
    class Runnable : public QRunnable {
        public:
            void run() override;
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

        bool event(QEvent* event) override;

    protected:
        void closeEvent(QCloseEvent * event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        QGlk(QWidget* parent = 0);

        bool handleGlkTask(Glk::TaskEvent* event);

        Ui::QGlk* ui;
        Glk::Runnable* mp_Runnable;
        Glk::Window* mp_RootWindow;
        Glk::EventQueue m_EventQueue;
        
        std::function<void(void)> m_InterruptHandler;
        
        Glk::StyleManager m_DefaultStyles;
        Glk::StyleManager m_TextBufferStyles;
};

#endif // QGLK_H
