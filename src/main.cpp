#include <exception>

#include <QApplication>
#include <QCloseEvent>

#include "glk.hpp"

#include "qglk.hpp"

QGlk* s_MainWindow = NULL;

QGlk& QGlk::getMainWindow() {
    return (*s_MainWindow);
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    QEvent::registerEventType(Glk::TaskEvent::Type);

    QGlk w(argc, argv);
    w.show();

    s_MainWindow = &w;

    w.run();

    return app.exec();
}

