#include <QApplication>
#include <ElaApplication.h>
#include <ElaWindow.h>
#include "ui/mainwindow.h"
#include "utils/logger.h"
#include "types/simname.h"
#include "types/color.h"

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
    // Init config
    if (!QFile::exists(config.getConfigPath())) {
        config.writeConfig();
    }
    config.readConfig();

    ElaApplication::getInstance()->init();
    // Init GDM
    GDM.setString(GDIN::NPF_FILE_PATH, "");
    GDM.setBool(GDIN::IS_OPEN_NPF, false);
    // Init logger
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
Version 1.3.0.0 TO-DO
TODO: 添加按Tab切换页面的功能
TODO: 添加启动动画，可关闭
TODO: 添加英语模式
TODO: 添加对打包日志进行备注的功能
TODO: 添加隐藏控制台选项的功能
TODO: 添加自定义指令的功能
TODO: 每次打包递增版本号，提供增加版本号的按钮
*/

/*
Version 1.3.0.0
重大修改：

普通修改：
1. 移除了导出页面，跟换为了打包日志页面
2. 移除了"工具"菜单
3. 将页面的选择栏移到了窗口上方
4. 采用ElaWidgetTools库美化并重构了ui
5. 添加了深色模式
6. 移除导入/导出页面
7. 移除了新建项目窗口
8. 添加了新建NPF时如果可以，自动填写项目信息的功能
9. 添加将打包/停止按钮吸附在其他窗口上的功能
10. 重构了关于(About)页面
11. 添加可以将文件拖动以打开的功能
修复的问题：
1. 修复了新建NPF文件时关闭窗口后会弹出导出文件选择窗口的问题
2. 修复了新建NPF文件时文件没有后缀的问题
*/

/*
代码方面的修改
重大修改：
1. 添加了ElaWidgetTools库
普通修改：
1. 分离了ProjectConfig类中关于ui的部分
*/
