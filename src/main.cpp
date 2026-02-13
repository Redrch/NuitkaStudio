#include <QApplication>
#include "ui/mainwindow.h"
#include "utils/logger.h"
#include "types/pcm.h"

bool isDebug = true;

void initProjectConfig() {
    // path data
    PCM::instance().addItem(new ProjectConfigType("pythonPath", QVariant("")));  // 0
    PCM::instance().addItem(new ProjectConfigType("mainFilePath", QVariant("")));  // 1
    PCM::instance().addItem(new ProjectConfigType("outputPath", QVariant("")));  // 2
    PCM::instance().addItem(new ProjectConfigType("outputFilename", QVariant("")));  // 3
    PCM::instance().addItem(new ProjectConfigType("projectPath", QVariant("")));  // 4
    PCM::instance().addItem(new ProjectConfigType("projectName", QVariant("")));  // 5
    PCM::instance().addItem(new ProjectConfigType("iconPath", QVariant("")));  // 6
    // bool data
    PCM::instance().addItem(new ProjectConfigType("standalone", QVariant(true)));  // 7
    PCM::instance().addItem(new ProjectConfigType("onefile", QVariant(false)));  // 8
    PCM::instance().addItem(new ProjectConfigType("removeOutput", QVariant(false)));  // 9
    // lto mode
    PCM::instance().addItem(new ProjectConfigType("ltoMode", QVariant::fromValue(LTOMode::Auto)));  // 10
    // data list
    PCM::instance().addItem(new ProjectConfigType("dataList", QVariant(QStringList())));  // 11
    // file data
    PCM::instance().addItem(new ProjectConfigType("fileVersion", QVariant("")));  // 12
    PCM::instance().addItem(new ProjectConfigType("company", QVariant("")));  // 13
    PCM::instance().addItem(new ProjectConfigType("productName", QVariant("")));  // 14
    PCM::instance().addItem(new ProjectConfigType("productVersion", QVariant("")));  // 15
    PCM::instance().addItem(new ProjectConfigType("fileDescription", QVariant("")));  // 16
    PCM::instance().addItem(new ProjectConfigType("legalCopyright", QVariant("")));  // 17
    PCM::instance().addItem(new ProjectConfigType("legalTrademarks", QVariant("")));  // 18
}

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  // 启动高DPI缩放
    qRegisterMetaType<LTOMode>("LTOMode");
    qRegisterMetaTypeStreamOperators<LTOMode>("LTOMode");
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
TODO: 添加打包日志功能，数据存储在npf文件中
TODO: 添加新增项的配置
TODO: 添加最小化到系统托盘的功能
TODO: 添加将打包/停止按钮吸附在其他窗口上的功能
TODO: 美化ui（优先级最低）
暂时先写这么多
*/

/*
重大修改：
1. 修改了版本号命名方式，由x.x.x改为了x.x.x.x
2. 修改了NPF文件格式，从二进制改为zip格式
3. 可以使用PCM代替ProjectConfigManager，使代码更加简洁

普通修改：
1. 填写项目路径，自动填写其他路径
2. 将项目配置的存储方式从struct改为了PCM
3. 添加程序信息配置
4. 新建项目时自动安装nuitka
5. 优化了使用UV新建项目的体验
6. 重构打包，设置UI界面

修复的问题：
修复了导出数据列表窗口会存在空项的问题
修复了导入新版NPF文件会崩溃的问题
*/
