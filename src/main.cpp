#include <QApplication>
#include "ui/mainwindow.h"
#include "utils/logger.h"
#include "types/simname.h"

bool isDebug = true;

void initProjectConfig() {
    // path data
    PCM.addItem(new ProjectConfigType("pythonPath", QVariant("")));  // 0
    PCM.addItem(new ProjectConfigType("mainFilePath", QVariant("")));  // 1
    PCM.addItem(new ProjectConfigType("outputPath", QVariant("")));  // 2
    PCM.addItem(new ProjectConfigType("outputFilename", QVariant("")));  // 3
    PCM.addItem(new ProjectConfigType("projectPath", QVariant("")));  // 4
    PCM.addItem(new ProjectConfigType("projectName", QVariant("")));  // 5
    PCM.addItem(new ProjectConfigType("iconPath", QVariant("")));  // 6
    // bool data
    PCM.addItem(new ProjectConfigType("standalone", QVariant(true)));  // 7
    PCM.addItem(new ProjectConfigType("onefile", QVariant(false)));  // 8
    PCM.addItem(new ProjectConfigType("removeOutput", QVariant(false)));  // 9
    // lto mode
    PCM.addItem(new ProjectConfigType("ltoMode", QVariant::fromValue(LTOMode::Auto)));  // 10
    // data list
    PCM.addItem(new ProjectConfigType("dataList", QVariant(QStringList())));  // 11
    // file data
    PCM.addItem(new ProjectConfigType("fileVersion", QVariant("")));  // 12
    PCM.addItem(new ProjectConfigType("company", QVariant("")));  // 13
    PCM.addItem(new ProjectConfigType("productName", QVariant("")));  // 14
    PCM.addItem(new ProjectConfigType("productVersion", QVariant("")));  // 15
    PCM.addItem(new ProjectConfigType("fileDescription", QVariant("")));  // 16
    PCM.addItem(new ProjectConfigType("legalCopyright", QVariant("")));  // 17
    PCM.addItem(new ProjectConfigType("legalTrademarks", QVariant("")));  // 18
}

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // 启动高DPI缩放
    qRegisterMetaType<LTOMode>("LTOMode");
    qRegisterMetaTypeStreamOperators<LTOMode>("LTOMode");
    qRegisterMetaType<EncodingEnum>("EncodingEnum");
    initProjectConfig();

    QApplication a(argc, argv);
    // init logger
    Logger::Config cfg;
    cfg.file_path = "app.log";
    Logger logger(cfg);
    if (!isDebug) Logger::installQtMessageHandler();
    // init mainwindow
    MainWindow w;
    w.show();
    // clean
    int ret = QApplication::exec();
    if (!isDebug) Logger::uninstallQtMessageHandler();
    logger.shutdown();
    return ret;
}


/*
Version 1.2.0.0 TO-DO
TODO: 添加将打包/停止按钮吸附在其他窗口上的功能
TODO: 美化ui（优先级最低）
暂时先写这么多
*/

/*
重大修改：
1. 修改了版本号命名方式，由x.x.x改为了x.x.x.x
2. 修改了NPF文件格式，从二进制改为zip格式
3. 可以使用PCM代替ProjectConfigManager::instance()，使代码更加简洁

普通修改：
1. 填写项目路径，自动填写其他路径
2. 将项目配置的存储方式从struct改为了PCM
3. 添加程序信息配置
4. 新建项目时自动安装nuitka
5. 优化了使用UV新建项目的体验
6. 重构打包，设置UI界面
7. 添加了全局变量管理类GlobalData
8. 修改了软件的标题格式，让其可以根据NPF文件路径显示
9. 新增了types/simname.h文件，可以通过此文件定义一些常用数据的简写
10. 重构了Config类
11. 添加打包日志功能，数据存储在npf文件中
12. 添加最小化到系统托盘的功能

修复的问题：
1. 修复了导出数据列表窗口会存在空项的问题
2. 修复了导入新版NPF文件会崩溃的问题
*/
