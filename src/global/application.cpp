//
// Created by redrch on 2026/4/15.
//

#include "application.h"
#include "types/simname.h"
#include "ui/mainwindow.h"
#include "update_clock.h"

#include <QSplashScreen>
#include <QApplication>
#include <QThread>

#include <ElaApplication.h>
#include <spdlog/common.h>

Application::Application() = default;

void Application::init() {
    // UI Init
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // 启动高DPI缩放
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // Register Type
    qRegisterMetaType<PackLog>("PackLog");
    qRegisterMetaType<LTOMode>("LTOMode");
    qRegisterMetaType<Encoding>("ConfigEnumNS::Encoding");
    qRegisterMetaType<Language>("ConfigEnumNS::Language");
    qRegisterMetaTypeStreamOperators<Encoding>("ConfigEnumNS::Encoding");
    qRegisterMetaTypeStreamOperators<Language>("ConfigEnumNS::Language");
    qRegisterMetaTypeStreamOperators<LTOMode>("LTOMode");

    // Init logger
    Logger::Config cfg;
    cfg.file_path = "app.log";
#ifndef QT_DEBUG
    cfg.level = spdlog::level::info;
#endif
    this->logger = new Logger(cfg);
#ifndef QT_DEBUG
    Logger::installQtMessageHandler();
#endif

    // Init PCM
#pragma region InitPCMSection
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
#pragma endregion

    // Init config
    config.init();
    if (!QFile::exists(config.getConfigPath())) {
        config.writeConfig();
    }
    config.readConfig();
    Logger::info("配置载入完成");
}

void Application::run() {
    // Start update clock
    UpdateClock::instance().start();
    // Splash screen
    QPixmap pixmap(":/logo");
    this->splash = new QSplashScreen(pixmap);
    if (config.getBool(ConfigItem::IsSplashScreen)) {
        splash->show();
        qApp->processEvents();
    }

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

    // Init ElaWidgetTools library
    ElaApplication::getInstance()->init();

    this->mainWindow = new MainWindow();
    QThread::msleep(500); // 仅用于让开屏动画可以正常显示，不至于一闪而过
    this->mainWindow->show();
    this->splash->finish(this->mainWindow);
}

void Application::exit() const {
    // clean logger
#ifndef QT_DEBUG
    Logger::uninstallQtMessageHandler();
#endif
    this->logger->shutdown();
    // delete translator object
    QTranslator* translator = GDM.get(GDIN::translator).value<QTranslator*>();
    delete translator;
    qApp->quit();
}

void Application::restart() const {
    this->exit();
    QProcess::execute("cmd /c \"ping 127.0.0.1 -n 2 > nul && start NuitkaStudio.exe\"");
}

void Application::changeLanguage(const Language &language, bool isRestart) const {
    config.setLanguage(ConfigItem::Language, language);
    config.writeConfig();
    if (isRestart) {
        QTimer::singleShot(0, [this]() { this->restart(); });
    }
}
