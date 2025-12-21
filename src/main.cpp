#include <QApplication>
#include "mainwindow.h"
#include "logger.h"

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // 启动高DPI缩放

    QApplication a(argc, argv);
    // init logger
    Logger::Config cfg;
    cfg.file_path = "app.log";
    Logger logger(cfg);
    Logger::installQtMessageHandler();
    // init mainwindow
    MainWindow w;
    w.show();
    // clean
    int ret = QApplication::exec();
    Logger::uninstallQtMessageHandler();
    logger.shutdown();
    return ret;
}
