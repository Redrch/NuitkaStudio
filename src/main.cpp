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
Version 1.4.0.0 TO-DO
*/

/**
Version 1.4.0.0
功能方面：
重大修改：
1. 重构 UI
普通修改：
1. 添加了外观设置
2. 添加了可以将图片作为背景的功能
代码方面：
重大修改：
1. 增加了编译时如果工具不存在自动安装的功能
普通修改：
1. 修改了PCM的接口名称
2. 添加了PCM自动刷新ui的功能
3. 简化了cmake脚本
4. 加入了ui刷新时钟(UpdateClock)
*/
