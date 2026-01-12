#include <QApplication>
#include "ui/mainwindow.h"
#include "utils/logger.h"

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


/*
Version 1.2.0 TO-DO
TODO: 自动搜索Python解释器路径
TODO: 自动配置输出路径
TODO: 添加程序信息配置
TODO: 新建项目时自动安装nuitka
TODO: 重构打包UI界面
TODO: 重构设置UI页面（如果需要）
TODO: 添加打包日志功能，数据存储在npf文件中
TODO: 添加新增项的配置
TODO: 添加最小化到系统托盘的功能
TODO: 添加将打包/停止按钮吸附在其他窗口上的功能
TODO: 美化ui（优先级最低）
暂时先写这么多
*/
