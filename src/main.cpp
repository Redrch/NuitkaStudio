#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // 启动高DPI缩放

    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return QApplication::exec();
}
