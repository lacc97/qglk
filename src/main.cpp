#include <exception>

#include <QApplication>
#include <QCloseEvent>

#include <spdlog/spdlog.h>

#include "glk.hpp"

#include "qglk.hpp"

QGlk* s_MainWindow = NULL;

QGlk& QGlk::getMainWindow() {
    return (*s_MainWindow);
}

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    spdlog::set_pattern("[%Y-%m-%d %T.%e] [%^%L%$] %v");


    QApplication app(argc, argv);

    QEvent::registerEventType(Glk::TaskEvent::Type);

    QGlk w(argc, argv);
    w.show();

    s_MainWindow = &w;

    w.run();

    return app.exec();
}

