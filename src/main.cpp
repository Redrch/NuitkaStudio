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
*/
