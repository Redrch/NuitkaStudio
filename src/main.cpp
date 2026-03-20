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

bool isDebug = true;

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

    initProjectConfig();

    QApplication a(argc, argv);
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
    if (!isDebug) Logger::installQtMessageHandler();

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
    if (!isDebug) Logger::uninstallQtMessageHandler();
    logger.shutdown();
    GDM.get(GDIN::translator).value<QTranslator*>()->deleteLater();
    return ret;
}

/*
Version 1.3.0.0 TO-DO
*/

/**
Version 1.3.0.0
重大修改：
1. 添加英语模式
2. NPF文件改用了NPF Version 2格式
3. 采用ElaWidgetTools库美化并重构了ui
普通修改：
1. 移除了导出页面，跟换为了打包日志页面
2. 移除了"工具"菜单
3. 将页面的选择栏移到了窗口上方
4. 添加了深色模式
5. 移除导入/导出页面
6. 移除了新建项目窗口
7. 添加了新建NPF时如果可以，自动填写项目信息的功能
8. 添加将打包/停止按钮吸附在其他窗口上的功能
9. 重构了关于(About)页面
10. 添加可以将文件拖动以打开的功能
11. 添加按Tab切换页面的功能
12. 添加启动动画，可关闭
13. 将打包计时器的位置改到了底部中间位置
14. 添加对打包日志进行备注的功能
15. 添加隐藏窗体的功能
16. 添加自定义指令的功能
17. 添加增加版本号的按钮

修复的问题：
1. 修复了新建NPF文件时关闭窗口后会弹出导出文件选择窗口的问题
2. 修复了新建NPF文件时文件没有后缀的问题
3. 修复了打包时有时会提示 “警告：原 NPF 文件内容解压失败或为空！归档已取消以保护原文件。” 的问题

代码方面的修改
重大修改：
1. 添加了ElaWidgetTools库
2. 添加了EventBus用于广播信号
普通修改：
1. 分离了ProjectConfig类中关于ui的部分
2. 重构了Config类
3. 将SettingsEnum枚举的名称改为了ConfigItem
4. 在Utils类中添加了通过QMetaType将enum转换为string或者int的函数
5. 修改了Config类api接口的名称
6. 修改了菜单栏信号的连接方式
*/
