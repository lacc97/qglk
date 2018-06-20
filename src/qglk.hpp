#ifndef QGLK_H
#define QGLK_H

#include <QMainWindow>
#include <QWidget>

#include "event/eventqueue.hpp"
#include "thread/taskrequest.hpp"
#include "window/window.hpp"

namespace Ui {
    class QGlk;
}

class QGlk;

class QGlk : public QMainWindow {
        Q_OBJECT

        friend int main(int, char**);
    public:
        static QGlk& getMainWindow();

        ~QGlk();

        inline Glk::Window* rootWindow() const {
            return mp_RootWindow;
        }
        inline void setRootWindow(Glk::Window* win) {
            mp_RootWindow = win;
            setCentralWidget(mp_RootWindow ? mp_RootWindow : new QWidget(this));
        }

        inline Glk::EventQueue& eventQueue() {
            return m_EventQueue;
        }

        bool event(QEvent* event) override;

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        QGlk(QWidget* parent = 0);

        bool handleGlkTask(Glk::TaskEvent* event);

        Ui::QGlk* ui;
        Glk::Window* mp_RootWindow;
        Glk::EventQueue m_EventQueue;
};

#endif // QGLK_H
