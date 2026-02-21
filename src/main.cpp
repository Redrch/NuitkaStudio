#include <QApplication>
#include <ElaApplication.h>
#include <ElaWindow.h>
#include "ui/mainwindow.h"
#include "utils/logger.h"
#include "types/simname.h"
#include "types/color.h"

bool isDebug = false;

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
    // init config
    if (!QFile::exists(config.getConfigPath())) {
        config.writeConfig();
    }
    config.readConfig();

    ElaApplication::getInstance()->init();
    // init logger
    Logger::Config cfg;
    cfg.file_path = "app.log";
    Logger logger(cfg);
    if (!isDebug) Logger::installQtMessageHandler();

    // load qss
    QFile qssFile(":/assets/style.qss");
    if (qssFile.open(QIODevice::ReadOnly)) {
        QString styleSheet = qssFile.readAll();
        for (auto it = color.begin(); it != color.end(); ++it) {
            styleSheet.replace(it.key(), it.value());
        }
        a.setStyleSheet(styleSheet);
        Logger::info("加载QSS成功");
    } else {
        Logger::error("加载QSS失败");
    }

    // if (config.getConfigToBool(SettingsEnum::IsLightMode)) {
    //     QFile qssFile(":/qdarkstyle/light/lightstyle.qss");
    //     if (qssFile.open(QIODevice::ReadOnly)) {
    //         QString styleSheet = qssFile.readAll();
    //         a.setStyleSheet(styleSheet);
    //         qssFile.close();
    //         Logger::info("加载QLightStyle成功");
    //     } else {
    //         Logger::error("无法加载QLightStyle");
    //     }
    // } else {
    //     QFile qssFile(":/qdarkstyle/dark/darkstyle.qss");
    //     if (qssFile.open(QIODevice::ReadOnly)) {
    //         QString styleSheet = qssFile.readAll();
    //         a.setStyleSheet(styleSheet);
    //         qssFile.close();
    //         Logger::info("加载QDarkStyle成功");
    //     } else {
    //         Logger::error("无法加载QDarkStyle");
    //     }
    // }

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
Version 1.3.0.0 TO-DO
TODO: 美化ui
TODO: 添加将打包/停止按钮吸附在其他窗口上的功能
TODO: 添加可以将文件拖动以打开的功能
TODO: 添加按Tab切换页面的功能
TODO: 添加启动动画，可关闭
TODO: 添加英语模式
TODO: 添加对打包日志进行备注的功能
TODO: 移除导入/导出页面
暂时先写这么多
*/

/*
Version 1.3.0.0
重大修改：

普通修改：

修复的问题：

*/

/*
代码方面的修改
重大修改：

普通修改：
*/
