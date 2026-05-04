//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "global/update_clock.h"
#include "global/application.h"
#include "styles/status_bar_style.h"

#include <ElaStatusBar.h>
#include <ElaMenuBar.h>
#include <ElaMenu.h>
#include <ElaContentDialog.h>
#include <ElaApplication.h>

MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent) {
    this->setAcceptDrops(true);
    this->setFocusPolicy(Qt::StrongFocus);
    this->setWindowTitle("Nuitka Studio");
    this->setWindowIcon(QIcon(":/logo"));
    this->moveToCenter();
    qApp->installEventFilter(this);
    this->currentPageIndex = 0;
    this->currentPackLogIndex = 0;

    // set window flags
    this->setWindowButtonFlag(ElaAppBarType::NavigationButtonHint, false);
    this->setWindowButtonFlag(ElaAppBarType::RouteBackButtonHint, false);
    this->setWindowButtonFlag(ElaAppBarType::RouteForwardButtonHint, false);

    // init objects
    this->packTimer = new QTimer(this);
    this->packLog = new QList<PackLogStruct *>();
    this->mainTimer = new QTimer(this);
    this->mainTimer->start(1);

    this->packLogModel = new QStringListModel(this);
    this->dataListModel = new QStringListModel(this);

    // Create the temp path
    if (!QDir(config.getString(ConfigItem::TempPath)).exists()) {
        if (!QDir().mkpath(config.getString(ConfigItem::TempPath))) {
            Logger::warn("缓存文件夹创建失败");
        }
    }

    // open npf file
    if (!config.getString(ConfigItem::NpfPath).isEmpty()) {
        QString path = config.getString(ConfigItem::NpfPath);
        NPFStatusType status = ProjectConfig::loadProject(path);
        if (!this->npfStatusTypeHandler(status, path, false)) {
            if (!GDM.getString(GDIN::npfFilePath).isEmpty()) {
                this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
            }
            GDM.setBool(GDIN::isOpenNPF, true);
        }

        // note file
        this->noteFile = new QFile(
            GDM.getString(GDIN::packLogPath) + "/" + QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName() + "/note.json");
        if (!this->noteFile->open(QIODevice::ReadOnly)) {
            Logger::warn("无法打开note.json文件");
        }
        this->noteObject = QJsonDocument::fromJson(this->noteFile->readAll()).object();
        this->noteFile->close();
    }

    // Init UI
    this->initUI();
    this->updateUI();

    // Close Window Event
    this->closeDialog = new ElaContentDialog(this);
    connect(closeDialog, &ElaContentDialog::rightButtonClicked, this, &MainWindow::close);
    connect(closeDialog, &ElaContentDialog::middleButtonClicked, this, [=]() {
        closeDialog->close();
        showMinimized();
    });
    this->setIsDefaultClosed(false);
    connect(this, &MainWindow::closeButtonClicked, this, [=]() {
        closeDialog->exec();
    });

    // Connect signal and slot
    this->connectOther();

    if (!GDM.getBool(GDIN::isOpenNPF)) {
        this->showText(tr("请先新建或打开一个NPF文件再进行操作"), -1, Qt::red);
    }

    Logger::info("初始化MainWindow类完成");
}

MainWindow::~MainWindow() {
    config.writeConfig();
    this->log->saveNote();
    delete this->floatButton;
    for (PackLogStruct *log: *this->packLog) {
        delete log;
    }
    delete this->packLog;
    delete this->log;
}

void MainWindow::startPack() {
    this->startPackAction->setEnabled(false);
    this->stopPackAction->setEnabled(true);
    QElapsedTimer timer;

    QString nowString = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString logName = nowString + ".log";
    QString npfFileName = QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName();
    QString logDir = GDM.getString(GDIN::packLogPath) + "/" + npfFileName;
    QString logPath = logDir + "/" + logName;
    if (!QDir(logDir).exists()) {
        if (!QDir().mkpath(logDir)) {
            Logger::warn("打包日志文件夹创建失败");
        }
    }
    auto *logFile = new QFile(logPath);
    logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    this->noteObject.insert(logName, "");

    timer.start();
    this->packProcess = new QProcess(this);

    // Signals and slots
    // 合并普通和错误输出
    this->packProcess->setProcessChannelMode(QProcess::MergedChannels);
    // output
    connect(this->packProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString out = QString::fromLocal8Bit(this->packProcess->readAllStandardOutput());
        this->packPage->addConsoleContent(out);
        logFile->write(out.toUtf8());
        Logger::info(out);
    });
    // finished
    connect(this->packProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus != QProcess::NormalExit) {
                    return;
                }
                qint64 time = timer.elapsed();
                qint64 second = time / 1000;
                qint64 ms = time % 1000;
                QString timeString;
                if (second < 60) {
                    timeString = QString("%1秒%2毫秒").arg(second).arg(ms);
                } else {
                    qint64 minute = second / 60;
                    timeString = QString("%1分钟%2秒%3毫秒").arg(minute).arg(second - minute * 60).arg(ms);
                }

                QString endOutString = QString("----------- 打包结束 耗时: %1 ----------").arg(timeString);
                this->packPage->addConsoleContent(endOutString);
                Logger::info(QString("----------- 打包结束 耗时: %1 ----------").arg(timeString));
                this->showText(QString("打包结束 耗时: %1").arg(timeString), 5000, Qt::black,
                               TextPos::SystemMessage, "打包通知");
                this->floatButton->packFinished();
                this->packProcess->deleteLater();
                this->packTimer->stop();
                this->packPage->packEnd();

                if (logFile) {
                    logFile->flush();
                    logFile->close();
                    logFile->deleteLater();
                }

                this->log->saveNote();
            });
    // error occurred
    connect(this->packProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        qWarning() << "command error: " << error;
        this->packPage->addConsoleContent("Error: " + Utils::processErrorToString(error));
        logFile->write(QString("Error: " + Utils::processErrorToString(error)).toUtf8());
        Logger::error("Error: " + Utils::processErrorToString(error));
    });

    if (PCM.getString(PCE::PythonPath).isEmpty()) {
        this->packPage->addConsoleContent(tr("python解释器路径为必填项"));
        this->startPackAction->setEnabled(true);
        this->stopPackAction->setEnabled(false);
        this->packPage->packEnd();
        return;
    }
    if (PCM.getString(PCE::MainfilePath).isEmpty()) {
        this->packPage->addConsoleContent(tr("主文件路径为必填项"));
        this->startPackAction->setEnabled(true);
        this->stopPackAction->setEnabled(false);
        this->packPage->packEnd();
        return;
    }
    if (PCM.getString(PCE::OutputPath).isEmpty()) {
        this->packPage->addConsoleContent(tr("输出目录为必填项"));
        this->startPackAction->setEnabled(true);
        this->stopPackAction->setEnabled(false);
        this->packPage->packEnd();
        return;
    }
    if (PCM.getString(PCE::OutputFilename).isEmpty()) {
        this->packPage->addConsoleContent(tr("输出文件名为必填项"));
        this->startPackAction->setEnabled(true);
        this->stopPackAction->setEnabled(false);
        this->packPage->packEnd();
        return;
    }

    // build args
    QStringList args = QStringList();
    args << "-m" << "nuitka";
    if (PCM.getBool(PCE::Standalone)) {
        args << "--standalone";
    }
    if (PCM.getBool(PCE::Onefile)) {
        args << "--onefile";
    }
    if (PCM.getBool(PCE::RemoveOutput)) {
        args << "--remove-output";
    }

    // LTO
    switch (PCM.getItem(PCE::LtoMode)->get_itemValue().value<LTOMode>()) {
        case LTOMode::No:
            args << "--lto=no";
            break;
        case LTOMode::Yes:
            args << "--lto=yes";
            break;
        case LTOMode::Auto:
            args << "--lto=auto";
            break;
    }

    args << PCM.getString(PCE::MainfilePath);
    args << "--output-dir=" + PCM.getString(PCE::OutputPath);
    args << "--output-filename=" + PCM.getString(
        PCE::OutputFilename);

    if (!PCM.getString(PCE::IconPath).isEmpty()) {
        args << "--windows-icon-from-ico=" + PCM.getString(
            PCE::IconPath);
    }
    if (!PCM.getString(PCE::FileVersion).isEmpty()) {
        args << "--file-version=" + PCM.getString(
            PCE::FileVersion);
    }
    if (!PCM.getString(PCE::Company).isEmpty()) {
        args << "--company-name=" + PCM.getString(PCE::Company);
    }
    if (!PCM.getString(PCE::ProductName).isEmpty()) {
        args << "--product-name=" + PCM.getString(
            PCE::ProductName);
    }
    if (!PCM.getString(PCE::ProductVersion).isEmpty()) {
        args << "--product-version=" + PCM.getString(
            PCE::ProductVersion);
    }
    if (!PCM.getString(PCE::FileDescription).isEmpty()) {
        args << "--file-description=" + PCM.getString(
            PCE::FileDescription);
    }
    if (!PCM.getString(PCE::LegalCopyright).isEmpty()) {
        args << "--copyright=" + PCM.getString(
            PCE::LegalCopyright);
    }
    if (!PCM.getString(PCE::LegalTrademarks).isEmpty()) {
        args << "--trademarks=" + PCM.getString(
            PCE::LegalTrademarks);
    }
    if (!PCM.getString(PCE::CustomCommand).isEmpty()) {
        QString command = PCM.getString(PCE::CustomCommand);
        QStringList commandArgs = command.split(" ");
        args << commandArgs;
    }

    this->packProcess->start(
        PCM.getString(PCE::PythonPath), args);
    this->packPage->packStart();

    // console output
    QString outputString = QString("-------------- 开始打包 %1 -------------").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    this->packPage->addConsoleContent(outputString);
    // pack timer
    this->startPackTime = QDateTime::currentDateTime();
    this->packTimer->start(config.getInt(ConfigItem::PackTimerTriggerInterval));
    // console output
    this->packPage->addConsoleContent(PCM.getString(PCE::PythonPath) + " " + args.join(" "));
    Logger::info(
        "开始打包  打包命令: " + QString(
            PCM.getString(PCE::PythonPath) + " " + args.
            join(" ")));
}

void MainWindow::stopPack() {
    if (!this->packProcess) {
        this->packPage->addConsoleContent(tr("没有正在执行的打包任务"));
        return;
    }
    if (this->packProcess->state() == QProcess::NotRunning) {
        this->packPage->addConsoleContent(tr("打包任务已结束"));
        this->packProcess->deleteLater();
        this->packProcess = nullptr;
        return;
    }

    this->packPage->addConsoleContent(tr("正在停止打包任务"));
    this->packProcess->terminate();

    QTimer::singleShot(5000, this, [=]() {
        if (this->packProcess->state() != QProcess::NotRunning) {
            this->packPage->addConsoleContent(tr("进程未响应，强制终止中..."));
            this->packProcess->kill();
            this->packProcess->deleteLater();
        }
    });
    this->showText(tr("已停止打包任务"), 5000, Qt::black, TextPos::SystemMessage, tr("打包通知"));
    this->packPage->packEnd();

    this->packTimer->stop();
}

void MainWindow::importProject() {
    QString path = QFileDialog::getOpenFileName(this, "Nuitka Studio  导入项目文件",
                                                config.getString(ConfigItem::DefaultDataPath),
                                                "Nuitka Project File(*.npf);;All files(*)");
    if (path.isEmpty()) {
        return;
    }
    NPFStatusType status = ProjectConfig::loadProject(path);
    if (this->npfStatusTypeHandler(status, path)) {
        return;
    }
    // Update UI
    if (!path.isEmpty()) {
        this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
    }
    GDM.setBool(GDIN::isOpenNPF, true);
    this->clearText();
}

void MainWindow::exportProject() {
    QString path = QFileDialog::getSaveFileName(this, "Nuitka Studio  导出项目文件",
                                                config.getString(ConfigItem::DefaultDataPath),
                                                "Nuitka Project File(*.npf);;All files(*)");
    if (path.isEmpty()) {
        return;
    }
    NPFStatusType status = ProjectConfig::saveProject(path, false);
    if (this->npfStatusTypeHandler(status, path)) {
        return;
    }
    if (!path.isEmpty()) {
        this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
    }
    GDM.setBool(GDIN::isOpenNPF, true);
}

void MainWindow::connectOther() {
    connect(&GDM, &GlobalData::valueChanged, this, [=](const QString &valueName, const QVariant &newValue) {
        if (valueName == GDIN::isOpenNPF) {
            if (newValue.toBool()) {
                this->enableUi();
            } else {
                this->disableUi();
            }
        }
    });

    connect(this->floatButton, &FloatButton::startPack, this, &MainWindow::startPack);
    connect(this->floatButton, &FloatButton::stopPack, this, &MainWindow::stopPack);
    connect(this->floatButton, &FloatButton::showMainWindow, this, [=]() {
        this->floatButton->hide();
        this->showNormal();
        this->activateWindow();
    });

    // Pack Timer
    connect(this->packTimer, &QTimer::timeout, this, [=]() {
        auto now = QDateTime::currentDateTime();
        qint64 time = now.toMSecsSinceEpoch() - this->startPackTime.toMSecsSinceEpoch();
        QString timeString = Utils::formatMilliseconds(time);
        this->messageLabel->setText(timeString);
    });
}

void MainWindow::initUI() {
    // Navigation
#pragma region Navigation
    this->packPage = new PackPage(this);
    this->settingsPage = new SettingsPage(this);
    this->packLogPage = new PackLogPage(this);
    this->log = new PackLog(this->packLogPage, true);
    this->packLogPage->setPackLogObj(this->log);
    this->log->updateLog();

    QString packPageKey{};
    QString settingsPageKey{};
    this->addExpanderNode(tr("打包"), packPageKey,ElaIconType::BoxesPacking);
    this->addExpanderNode(tr("设置"), settingsPageKey, ElaIconType::Gear);

    this->addPageNode(tr("基础配置"), this->packPage, packPageKey, ElaIconType::Database);
    this->addPageNode(tr("打包配置"), this->packPage, packPageKey, ElaIconType::File);
    this->addPageNode(tr("资源配置"), this->packPage, packPageKey, ElaIconType::LinkHorizontal);
    this->addPageNode(tr("文件信息配置"), this->packPage, packPageKey, ElaIconType::FileCircleInfo);
    this->addPageNode(tr("控制台"), this->packPage, packPageKey, ElaIconType::Command);
    expandNavigationNode(packPageKey);

    this->addPageNode(tr("通用设置"), this->settingsPage, settingsPageKey, ElaIconType::GripLines);
    this->addPageNode(tr("外观设置"), this->settingsPage, settingsPageKey, ElaIconType::Airplay);
    this->addPageNode(tr("默认路径设置"), this->settingsPage, settingsPageKey, ElaIconType::AtomSimple);
    expandNavigationNode(settingsPageKey);

    this->addPageNode(tr("打包日志"), this->packLogPage, ElaIconType::File);

    this->setUserInfoCardPixmap(QPixmap(":/logo"));
    this->setUserInfoCardTitle("Nuitka Studio");
    this->setUserInfoCardSubTitle("Redrch");
    this->setNavigationBarWidth(250);

    connect(this, &ElaWindow::navigationNodeClicked, this,
        [=](ElaNavigationType::NavigationNodeType nodeType, QString nodeKey) {
            if (nodeType == ElaNavigationType::PageNode) {
                const QString title = this->getNavigationNodeTitle(nodeKey);
                const PageCard card = this->navigationTitleEnumMap.value(title);
                const int cardId = Utils::enumToInt(card);
                if (cardId < 0x0100) {
                    this->packPage->scrollTo(card);
                    Logger::debug("Navigation: 点击 PackPage 的子节点");
                } else if (cardId < 0x0200) {
                    this->settingsPage->scrollTo(card);
                    Logger::debug("Navigation: 点击 SettingsPage 的子节点");
                }
            }
        });
#pragma endregion

    // Status bar
#pragma region StatusBar
    ElaStatusBar *statusBar = new ElaStatusBar();
    statusBar->setStyle(new StatusBarStyle(statusBar->style()));
    this->setStatusBar(statusBar);
    this->messageLabel = new ElaText("", 9, this);
    this->messageLabel->setAlignment(Qt::AlignCenter);
    this->statusLabel = new ElaText("", 9, this);
    this->statusLabel->setVisible(false);
    this->statusFileNameLabel = new ElaText(QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName(), 9, this);
    ElaText *appNameLabel = new ElaText(QString("Nuitka Studio v%1").arg(APP_VERSION), 9, this);
    statusBar->addWidget(this->messageLabel);
    statusBar->addPermanentWidget(this->statusFileNameLabel);
    statusBar->addPermanentWidget(appNameLabel);
    statusBar->addPermanentWidget(this->statusLabel);
#pragma endregion

    // Menubar
#pragma region MenuBar
    ElaMenuBar *menuBar = new ElaMenuBar(this);

    // menus
    ElaMenu *fileMenu = new ElaMenu(tr("文件"), this);
    ElaMenu *packMenu = new ElaMenu(tr("打包"), this);
    ElaMenu *helpMenu = new ElaMenu(tr("帮助"), this);

    // actions
    // file menu
    QAction *newAction = new QAction(tr("新建"), this);
    newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    fileMenu->addAction(newAction);
    QAction *openAction = new QAction(tr("打开"), this);
    openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    fileMenu->addAction(openAction);
    QAction *saveAction = new QAction(tr("保存"), this);
    saveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    fileMenu->addAction(saveAction);
    QAction *saveAsAction = new QAction(tr("另存为"), this);
    saveAsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_S));
    fileMenu->addAction(saveAsAction);
    QAction *saveConfigAction = new QAction(tr("保存配置"), this);
    saveConfigAction->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
    fileMenu->addAction(saveConfigAction);
    QAction *closeAction = new QAction(tr("关闭文件"), this);
    fileMenu->addAction(closeAction);

    fileMenu->addSeparator();

    QAction *hideAction = new QAction(tr("隐藏"), this);
    hideAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_H));
    fileMenu->addAction(hideAction);
    QAction *floatAction = new QAction(tr("浮动按钮"), this);
    floatAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_F));
    fileMenu->addAction(floatAction);
    QAction *exitAction = new QAction(tr("退出"), this);
    fileMenu->addAction(exitAction);

    // pack menu
    QAction *startPackAction = new QAction(tr("开始打包"), this);
    startPackAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_S));
    packMenu->addAction(startPackAction);
    QAction *endPackAction = new QAction(tr("结束打包"), this);
    endPackAction->setShortcut(QKeySequence(Qt::ALT + Qt::Key_E));
    packMenu->addAction(endPackAction);

    // help menu
    QAction *aboutAction = new QAction(tr("关于"), this);
    helpMenu->addAction(aboutAction);
    QAction *helpAction = new QAction(tr("帮助"), this);
    helpMenu->addAction(helpAction);

    // add menus to menubar
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(packMenu);
    menuBar->addMenu(helpMenu);

    this->setMenuBar(menuBar);

    // connect signals and slots
    // file menu
    connect(newAction, &QAction::triggered, this, [=]() {
        QString path = QFileDialog::getSaveFileName(this, "Nuitka Studio 新建NPF文件",
                                                    config.getString(ConfigItem::DefaultDataPath),
                                                    "Nuitka Project File(*.npf);;All files(*)");
        if (path.isEmpty()) {
            return;
        }
        PCM.setDefaultValue();
        if (this->npfStatusTypeHandler(ProjectConfig::saveProject(path, config.getBool(ConfigItem::IsSavePackLog)),
                                       path)) {
            Logger::error("创建NPF文件错误");
            QMessageBox::critical(this, "Nuitka Studio Error", tr("创建NPF文件错误"));
            return;
        }
        GDM.setString(GDIN::npfFilePath, path);
        GDM.setBool(GDIN::isOpenNPF, true);
        QString dirPath = QFileInfo(path).absolutePath();
        QStringList entryList = QDir(dirPath).entryList();
        if (entryList.contains("src") || entryList.contains("main.py")) {
            int choose = QMessageBox::question(this, "Nuitka Studio",
                                               "检测到此目录是一个项目目录，是否自动填写参数（此判断有时会误判）");
            if (choose == QMessageBox::Yes) {
                PCM.set(PCE::ProjectPath, dirPath);
                this->genData();
            }
        }

        this->clearText();
    });
    connect(openAction, &QAction::triggered, this, [=]() {
        this->importProject();
        this->clearText();
        this->updateUI();
    });
    connect(saveAction, &QAction::triggered, this, [=]() {
        this->npfStatusTypeHandler(ProjectConfig::saveProject(GDM.getString(GDIN::npfFilePath),
                                                              config.getBool(ConfigItem::IsSavePackLog)),
                                   GDM.getString(GDIN::npfFilePath));
    });
    connect(saveAsAction, &QAction::triggered, this, &MainWindow::exportProject);
    connect(saveConfigAction, &QAction::triggered, this, [=]() {
        config.writeConfig();
    });
    connect(closeAction, &QAction::triggered, this, [=]() {
        int choose = QMessageBox::question(this, "Nuitka Studio", tr("关闭后未保存的数据将会丢失，是否确认关闭"));
        if (choose == QMessageBox::Yes) {
            PCM.setDefaultValue();
            GDM.setString(GDIN::npfFilePath, "");
            GDM.setBool(GDIN::isOpenNPF, false);
            config.setString(ConfigItem::NpfPath, "");
            this->updateUI();
        }
    });
    connect(hideAction, &QAction::triggered, this, &MainWindow::hide);
    connect(floatAction, &QAction::triggered, this, [=]() {
        this->hide();
        this->floatButton->show();
    });
    connect(exitAction, &QAction::triggered, this, [=]() {
        nApp.exit();
    });
    // pack menu
    connect(startPackAction, &QAction::triggered, this, &MainWindow::startPack);
    connect(endPackAction, &QAction::triggered, this, &MainWindow::stopPack);
    // help menu
    connect(aboutAction, &QAction::triggered, this, [=]() {
        AboutWindow *aboutWindow = new AboutWindow(this);
        aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
        aboutWindow->exec();
    });
    connect(helpAction, &QAction::triggered, this, [=]() {
        QDesktopServices::openUrl(QUrl("https://github.com/Redrch/NuitkaStudio"));
    });

#pragma endregion

    // Tray
#pragma region Tray
    this->trayIcon = new QSystemTrayIcon(QIcon(":/logo"), this);
    this->trayIcon->setToolTip("Nuitka Studio");

    this->trayMenu = new ElaMenu(this);
    this->startPackAction = new QAction(tr("开始打包项目"), this);
    this->stopPackAction = new QAction(tr("停止打包项目"), this);
    this->showAction = new QAction(tr("显示"), this);
    this->quitAction = new QAction(tr("退出"), this);
    this->stopPackAction->setEnabled(false);

    this->trayMenu->addAction(startPackAction);
    this->trayMenu->addAction(stopPackAction);
    this->trayMenu->addAction(showAction);
    this->trayMenu->addAction(quitAction);
    this->trayIcon->setContextMenu(trayMenu);

    // tray icon
    connect(this->trayIcon, &QSystemTrayIcon::activated, this, [=](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::DoubleClick) {
            this->showNormal();
            this->activateWindow();
        }
    });
    // start pack action
    connect(this->startPackAction, &QAction::triggered, this, [=]() {
        this->startPack();
    });
    // stop pack action
    connect(this->stopPackAction, &QAction::triggered, this, [=]() {
        this->stopPack();
    });
    // show action
    connect(this->showAction, &QAction::triggered, this, [=]() {
        this->showNormal();
        this->activateWindow();
        this->floatButton->hide();
    });
    // quit action
    connect(this->quitAction, &QAction::triggered, this, [=]() {
        qApp->quit();
    });

    this->trayIcon->show();
#pragma endregion

    // init top text label
    this->topTextLabel = new QLabel("", this);

    // lock pack ui
    if (!GDM.getBool(GDIN::isOpenNPF)) {
        this->disableUi();
    }

    // controls
    // float button
    PixmapGroup pg;
    pg.startLight = QPixmap(":/assets/start-light.png");
    pg.startDark = QPixmap(":/assets/start-dark.png");
    pg.stopLight = QPixmap(":/assets/stop-light.png");
    pg.stopDark = QPixmap(":/assets/stop-dark.png");
    this->floatButton = new FloatButton(pg, nullptr);
    this->floatButton->setObjectName("floatButton");
    this->floatButton->setWindowFlags(Qt::Widget | Qt::FramelessWindowHint | Qt::Tool | Qt::WindowStaysOnTopHint);
    this->floatButton->setAttribute(Qt::WA_TranslucentBackground);
    this->floatButton->hide();

    // window background
    this->setWindowPixmap(ElaThemeType::Light, config.getString(ConfigItem::PixmapPath, ConfigGroup::Appearance));
    this->setWindowPixmap(ElaThemeType::Dark, config.getString(ConfigItem::PixmapPath, ConfigGroup::Appearance));
    this->setWindowMoviePath(ElaThemeType::Light, config.getString(ConfigItem::MoviePath, ConfigGroup::Appearance));
    this->setWindowMoviePath(ElaThemeType::Dark, config.getString(ConfigItem::MoviePath, ConfigGroup::Appearance));
    this->setWindowPaintMode(static_cast<ElaWindowType::PaintMode>
        (Utils::enumToInt(config.getWindowBackground(ConfigItem::WindowBackground, ConfigGroup::Appearance))));

    connect(this->packPage, &PackPage::startPack, this, &MainWindow::startPack);
    connect(this->packPage, &PackPage::stopPack, this, &MainWindow::stopPack);
}

void MainWindow::updateUI() {
    this->statusFileNameLabel->setText(QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName());
    if (GDM.getBool(GDIN::isOpenNPF)) {
        this->setWindowTitle(QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName() + " - Nuitka Studio");
    }
    Theme theme = config.getTheme(ConfigItem::Theme, ConfigGroup::Appearance);
    if (theme == Theme::Light) {
        eTheme->setThemeMode(ElaThemeType::Light);
    } else if (theme == Theme::Dark) {
        eTheme->setThemeMode(ElaThemeType::Dark);
    }
}

// gen path functions
void MainWindow::genData(bool isUpdateUI) {
    if (PCM.getString(PCE::ProjectPath).isEmpty()) {
        QMessageBox::warning(this, "Nuitka Studio Warning", "请填写项目路径");
        return;
    }
    if (PCM.getString(PCE::ProjectName).isEmpty()) {
        PCM.set(PCE::ProjectName,
                    PCM.getString(
                        PCE::ProjectPath).split("/").last());
        if (PCM.getString(PCE::ProjectName).isEmpty()) {
            QMessageBox::warning(this, "Nuitka Studio Warning", "项目名为空且无法自动填写项目名");
            return;
        }
    }

    genPythonPath();
    genMainfilePath();
    genOutputPath();
    genOutputName();
    genFileInfo();
    if (isUpdateUI) {
        UpdateClock::instance().update();
    }
    Logger::info("生成数据");
}

void MainWindow::genPythonPath() {
    QDir projectDir(PCM.getString(PCE::ProjectPath));
    if (projectDir.exists(
        PCM.getString(PCE::ProjectPath) + "/.venv")) {
        PCM.set(PCE::PythonPath,
                    PCM.getString(
                        PCE::ProjectPath) + "/.venv" + "/Scripts" +
                    "/python.exe");
    } else {
        const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        QString path = env.value("PATH");
        QStringList parts = path.split(";");
        for (const QString &part: parts) {
            if (part.contains("python")) {
                QDir dir(part);
                QFileInfo fi(dir.filePath("python.exe"));
                if (fi.exists() && fi.isFile()) {
                    PCM.set(PCE::PythonPath, fi.filePath());
                }
            }
        }
    }
}

void MainWindow::genMainfilePath() {
    QDir projectDir(PCM.getString(PCE::ProjectPath));
    if (projectDir.exists(PCM.getString(PCE::ProjectPath) + "/src")
        || projectDir.exists(
            PCM.getString(PCE::ProjectPath) + "/source")) {
        PCM.set(PCE::MainfilePath,
                    PCM.getString(
                        PCE::ProjectPath) + "/src/main.py");
    } else {
        PCM.set(PCE::MainfilePath,
                    PCM.getString(
                        PCE::ProjectPath) +
                    "/main.py");
    }
}

void MainWindow::genOutputPath() {
    PCM.set(PCE::OutputPath,
                PCM.getString(
                    PCE::ProjectPath) +
                "/output");
}

void MainWindow::genOutputName() {
    PCM.set(PCE::OutputFilename,
                PCM.getString(
                    PCE::ProjectName) +
                ".exe");
}

void MainWindow::genFileInfo() {
    PCM.set(PCE::ProductName,
                PCM.getString(
                    PCE::ProjectName));
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    int x = (this->width() - this->topTextLabel->sizeHint().width()) / 2;
    int y = 30;
    this->topTextLabel->move(x, y);
    this->topTextLabel->raise();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    } else {
        event->ignore();
    }
}

void MainWindow::dropEvent(QDropEvent *event) {
    const QMimeData *mimeData = event->mimeData();

    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();

        for (const QUrl &url: urlList) {
            QString filePath = url.toLocalFile();
            QFileInfo pathInfo(filePath);
            if (!filePath.isEmpty()) {
                Logger::info("拖拽文件至窗口，文件路径：" + filePath);
                QString suffix = pathInfo.suffix().toLower();
                if (suffix == "npf") {
                    if (!this->npfStatusTypeHandler(ProjectConfig::loadProject(filePath), filePath)) {
                        this->genData();
                        this->clearText(TextPos::TopLabel);
                        this->setWindowTitle(pathInfo.fileName() + " - Nuitka Studio");
                    }
                } else if (suffix == "py") {
                    if (filePath.split("/").contains("src") || filePath.split("/").contains("source") ||
                        pathInfo.fileName() == "main.py") {
                        QString projectPath = pathInfo.absolutePath();
                        PCM.set(PCE::ProjectPath, projectPath);
                        this->genData();
                        this->clearText(TextPos::TopLabel);
                    }
                }
            }
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    }
    return QMainWindow::eventFilter(watched, event);
}

// ui utils functions
void MainWindow::showText(const QString &text, int showTime, const QColor &color, const TextPos position,
                          const QString &title) const {
    switch (position) {
        case TextPos::TopLabel:
            this->topTextLabel->setText(text);
            this->topTextLabel->setStyleSheet(QString("color: %1;").arg(color.name()));
            if (showTime >= 0) {
                QTimer::singleShot(showTime, this, [=]() {
                    this->topTextLabel->clear();
                });
            }
            this->topTextLabel->adjustSize();
            break;
        case TextPos::Statusbar:
            this->messageLabel->setText(text);
            this->messageLabel->setStyleSheet(QString("color: %1;").arg(color.name()));
            if (showTime >= 0) {
                QTimer::singleShot(showTime, this, [=]() {
                    this->messageLabel->clear();
                });
            }
            break;
        case TextPos::SystemMessage:
            QIcon logo(":/logo");
            if (showTime < 0) {
                showTime = 5000;
            }
            this->trayIcon->showMessage(title, text, logo, showTime);
            break;
    }
}

void MainWindow::clearText(TextPos position) const {
    switch (position) {
        case TextPos::TopLabel:
            this->topTextLabel->clear();
            break;
        case TextPos::Statusbar:
            this->messageLabel->clear();
            break;
        case TextPos::SystemMessage:
            break;
    }
}

void MainWindow::disableUi() const {
    this->packPage->setEnabled(false);
    this->packLogPage->setEnabled(false);
}

void MainWindow::enableUi() const {
    this->packPage->setEnabled(true);
    this->packLogPage->setEnabled(true);
}

// util functions
bool MainWindow::npfStatusTypeHandler(NPFStatusType status, const QString &path, bool isTip) {
    switch (status) {
        case NPFStatusType::NPFDamage:
            if (isTip) QMessageBox::critical(this, "Nuitka Studio Error", QString("npf文件%1已损坏，请尝试更换文件").arg(path));
            return true;
        case NPFStatusType::NPFVersionError:
            if (isTip) QMessageBox::critical(this, "Nuitka Studio Error", QString("npf文件%1的格式版本错误，请尝试更换文件").arg(path));
            return true;
        case NPFStatusType::NPFNotFound:
            if (isTip) QMessageBox::critical(this, "Nuitka Studio Error", QString("找不到npf文件%1").arg(path));
            return true;
        case NPFStatusType::NPFNotOpen:
            if (isTip) QMessageBox::critical(this, "Nuitka Studio Error", QString("无法打开npf文件"));
        case NPFStatusType::NotFoundNote:
            if (isTip) QMessageBox::critical(this, "Nuitka Studio Error", QString("note.json文件错误，请尝试不保存打包日志文件"));
        case NPFStatusType::NPFRight:
            return false;
    }
    return true;
}
