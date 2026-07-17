#include <QApplication>
#include "global/application.h"

int main(int argc, char *argv[]) {
    nApp.init();
    QApplication a(argc, argv);
    nApp.run();

    int ret = QApplication::exec();
    nApp.exit();
    return ret;
}

/*
Version 1.4.1 TO-DO
TODO: 添加引入模块/包的功能
TODO: 添加排除模块的功能
TODO: 添加Python FLAG
*/
