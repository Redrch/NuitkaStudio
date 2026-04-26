#include <QApplication>
#include "global/application.h"
#include "ui/settings_page.h"

int main(int argc, char *argv[]) {
    app.init();
    QApplication a(argc, argv);
    app.run();
    int ret = QApplication::exec();
    app.exit();
    return ret;
}

/*
Version 1.4.0.0 TO-DO
TODO: 添加更多选项与预设
TODO: 添加打包错误修复提示功能
*/

/**
Version 1.4.0.0
功能方面：
重大修改：
普通修改：
代码方面：
重大修改：
1. 增加了编译时如果工具不存在自动安装的功能
普通修改：
1. 修改了PCM的接口名称
2. 添加了PCM自动刷新ui的功能
3. 简化了cmake脚本
4. 加入了ui刷新时钟(UpdateClock)
*/
