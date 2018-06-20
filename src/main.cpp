#include <exception>

#include <QApplication>
#include <QCloseEvent>
#include <QRunnable>
#include <QThread>
#include <QThreadPool>

#include "glk.hpp"

#include "qglk.hpp"

QGlk* s_MainWindow = NULL;

QGlk& QGlk::getMainWindow() {
    return (*s_MainWindow);
}

namespace Glk {
    class ExitException : public std::exception {

    };
}

void glk_exit() {
    s_MainWindow->close();

    throw Glk::ExitException();
}
#include <iostream>
class GlkRunnable : public QRunnable {
    public:
        void run() override {
            try {
                glk_main();
            } catch(Glk::ExitException& ex) {}

            return;
        }
};

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QEvent::registerEventType(Glk::TaskEvent::Type);

    QGlk w;
    w.show();

    s_MainWindow = &w;

    GlkRunnable glkrun;
    QThreadPool::globalInstance()->start(&glkrun);

    return app.exec();
}

