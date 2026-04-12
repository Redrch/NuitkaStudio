#include <QApplication>
#include <QSplashScreen>
#include <QThread>
#include <QLocale>
#include <QTranslator>

#include <ElaApplication.h>
#include <ElaWindow.h>
#include "ui/mainwindow.h"
#include "utils/logger.h"
#include "types/simname.h"
#include "types/color.h"

void initProjectConfig() {
    // path data
    PCM.addItem(new ProjectConfigType("pythonPath", QVariant("")));                  // 0
    PCM.addItem(new ProjectConfigType("mainFilePath", QVariant("")));                // 1
    PCM.addItem(new ProjectConfigType("outputPath", QVariant("")));                  // 2
    PCM.addItem(new ProjectConfigType("outputFilename", QVariant("")));              // 3
    PCM.addItem(new ProjectConfigType("projectPath", QVariant("")));                 // 4
    PCM.addItem(new ProjectConfigType("projectName", QVariant("")));                 // 5
    PCM.addItem(new ProjectConfigType("iconPath", QVariant("")));                    // 6
    // bool data
    PCM.addItem(new ProjectConfigType("standalone", QVariant(true)));                // 7
    PCM.addItem(new ProjectConfigType("onefile", QVariant(false)));                  // 8
    PCM.addItem(new ProjectConfigType("removeOutput", QVariant(false)));             // 9
    // lto mode
    PCM.addItem(new ProjectConfigType("ltoMode", QVariant::fromValue(LTOMode::Auto)));       // 10
    // data list
    PCM.addItem(new ProjectConfigType("dataList", QVariant(QStringList())));    // 11
    // file data
    PCM.addItem(new ProjectConfigType("fileVersion", QVariant("")));                 // 12
    PCM.addItem(new ProjectConfigType("company", QVariant("")));                     // 13
    PCM.addItem(new ProjectConfigType("productName", QVariant("")));                 // 14
    PCM.addItem(new ProjectConfigType("productVersion", QVariant("")));              // 15
    PCM.addItem(new ProjectConfigType("fileDescription", QVariant("")));             // 16
    PCM.addItem(new ProjectConfigType("legalCopyright", QVariant("")));              // 17
    PCM.addItem(new ProjectConfigType("legalTrademarks", QVariant("")));             // 18
    // custom commands
    PCM.addItem(new ProjectConfigType("customCommand", QVariant("")));               // 19
}

int main(int argc, char *argv[]) {
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // 启动高DPI缩放
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    qRegisterMetaType<LTOMode>("LTOMode");
    qRegisterMetaTypeStreamOperators<LTOMode>("LTOMode");

    qRegisterMetaType<EncodingEnum>("ConfigEnumNS::EncodingEnum");
    qRegisterMetaType<Language>("ConfigEnumNS::Language");
    qRegisterMetaTypeStreamOperators<Language>("ConfigEnumNS::Language");

    qRegisterMetaType<PackLog>("PackLog");

    QApplication a(argc, argv);

    initProjectConfig();
    // Init config
    if (!QFile::exists(config.getConfigPath())) {
        config.writeConfig();
    }
    config.readConfig();

    // 开屏动画
    QPixmap pixmap(":/logo");
    QSplashScreen splash(pixmap);
    if (config.getBool(ConfigItem::IsSplashScreen)) {
        splash.show();
        a.processEvents();
    }

    // Init logger
    Logger::Config cfg;
    cfg.file_path = "app.log";
    Logger logger(cfg);
#ifndef QT_DEBUG
    Logger::installQtMessageHandler();
#endif

    // Init GDM
    GDM.setString(GDIN::npfFilePath, "");
    GDM.setBool(GDIN::isOpenNPF, false);
    GDM.set(GDIN::translator, QVariant::fromValue(new QTranslator));
    GDM.setString(GDIN::packLogPath, QApplication::applicationDirPath() + "/pack_log");

    // Load qm file
    if (GDM.get(GDIN::translator).value<QTranslator*>()->load(
        QString(":/lang/%1.qm").arg(Utils::enumToString(config.getLanguage(ConfigItem::Language))))) {
        QApplication::installTranslator(GDM.get(GDIN::translator).value<QTranslator*>());
        Logger::info("语言文件加载完毕");
    } else {
        Logger::error("无法加载语言文件");
    }

    ElaApplication::getInstance()->init();

    // init mainwindow
    MainWindow w;
    QThread::msleep(500); // 仅用于让开屏动画可以正常显示，不至于一闪而过
    w.show();
    splash.finish(&w);

    // load
    int ret = QApplication::exec();

    // clean
#ifndef QT_DEBUG
    Logger::uninstallQtMessageHandler();
#endif

    logger.shutdown();
    GDM.get(GDIN::translator).value<QTranslator*>()->deleteLater();
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
普通修改：
1. 修改了PCM的接口名称
2. 添加了PCM自动刷新ui的功能
*/
