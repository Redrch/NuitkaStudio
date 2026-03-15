//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->setAcceptDrops(true);
    this->setFocusPolicy(Qt::StrongFocus);
    qApp->installEventFilter(this);
    this->currentPageIndex = 0;
    this->currentPackLogIndex = 0;

    // set window flags
    this->setWindowButtonFlag(ElaAppBarType::NavigationButtonHint, false);
    this->setWindowButtonFlag(ElaAppBarType::RouteBackButtonHint, false);
    this->setWindowButtonFlag(ElaAppBarType::RouteForwardButtonHint, false);

    // init objects
    this->packTimer = new QTimer(this);
    this->packLog = new QList<PackLog *>();
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

    // Init translator string
    this->controlText = new ControlText();

    // Init UI
    this->initUI();

    // Connect signal and slot
    this->connectStackedWidget();
    this->connectMenubar();
    this->connectPackPage();
    this->connectSettingsPage();
    this->connectPackLog();
    this->connectTrayMenu();
    this->connectOther();

    if (!GDM.getBool(GDIN::isOpenNPF)) {
        this->showText(tr("请先新建或打开一个NPF文件再进行操作"), -1, Qt::red);
    }

    this->updateUI();

    Logger::info("初始化MainWindow类完成");
}

MainWindow::~MainWindow() {
    config.writeConfig();
    delete this->floatButton;
    for (PackLog *log: *this->packLog) {
        delete log;
    }
    delete this->packLog;
    delete ui;
}

void MainWindow::startPack() {
    ui->startPackBtn->setEnabled(false);
    this->startPackAction->setEnabled(false);
    ui->stopPackBtn->setEnabled(true);
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
        ui->consoleOutputEdit->appendPlainText(out);
        logFile->write(out.toUtf8());
        Logger::info(out);
    });
    // finished
    connect(this->packProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
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
                ui->consoleOutputEdit->appendPlainText(endOutString);
                Logger::info(QString("----------- 打包结束 耗时: %1 ----------").arg(timeString));
                this->showText(QString("打包结束 耗时: %1").arg(timeString), 5000, Qt::black,
                               TextPos::SystemMessage, "打包通知");
                this->floatButton->packFinished();
                this->packProcess->deleteLater();
                this->packTimer->stop();
                ui->startPackBtn->setEnabled(true);
                this->startPackAction->setEnabled(true);
                ui->stopPackBtn->setEnabled(false);
                this->stopPackAction->setEnabled(false);

                if (logFile) {
                    logFile->flush();
                    logFile->close();
                    logFile->deleteLater();
                }

                this->saveNote();
            });
    // error occurred
    connect(this->packProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        qWarning() << "command error: " << error;
        ui->consoleOutputEdit->appendPlainText("Error: " + Utils::processErrorToString(error));
        logFile->write(QString("Error: " + Utils::processErrorToString(error)).toUtf8());
        Logger::error("Error: " + Utils::processErrorToString(error));
    });

    if (PCM.getItemValueToString(PCE::PythonPath).isEmpty()) {
        ui->consoleOutputEdit->appendPlainText("python解释器路径为必填项");
        ui->startPackBtn->setEnabled(true);
        this->startPackAction->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        this->stopPackAction->setEnabled(false);
        return;
    }
    if (PCM.getItemValueToString(PCE::MainfilePath).isEmpty()) {
        ui->consoleOutputEdit->appendPlainText("主文件路径为必填项");
        ui->startPackBtn->setEnabled(true);
        this->startPackAction->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        this->stopPackAction->setEnabled(false);
        return;
    }
    if (PCM.getItemValueToString(PCE::OutputPath).isEmpty()) {
        ui->consoleOutputEdit->appendPlainText("输出目录为必填项");
        ui->startPackBtn->setEnabled(true);
        this->startPackAction->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        this->stopPackAction->setEnabled(false);
        return;
    }
    if (PCM.getItemValueToString(PCE::OutputFilename).isEmpty()) {
        ui->consoleOutputEdit->appendPlainText("输出文件名为必填项");
        ui->startPackBtn->setEnabled(true);
        this->startPackAction->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        this->stopPackAction->setEnabled(false);
        return;
    }

    // build args
    QStringList args = QStringList();
    args << "-m" << "nuitka";
    if (PCM.getItemValueToBool(PCE::Standalone)) {
        args << "--standalone";
    }
    if (PCM.getItemValueToBool(PCE::Onefile)) {
        args << "--onefile";
    }
    if (PCM.getItemValueToBool(PCE::RemoveOutput)) {
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

    args << PCM.getItemValueToString(PCE::MainfilePath);
    args << "--output-dir=" + PCM.getItemValueToString(PCE::OutputPath);
    args << "--output-filename=" + PCM.getItemValueToString(
        PCE::OutputFilename);

    if (!PCM.getItemValueToString(PCE::IconPath).isEmpty()) {
        args << "--windows-icon-from-ico=" + PCM.getItemValueToString(
            PCE::IconPath);
    }
    if (!PCM.getItemValueToString(PCE::FileVersion).isEmpty()) {
        args << "--file-version=" + PCM.getItemValueToString(
            PCE::FileVersion);
    }
    if (!PCM.getItemValueToString(PCE::Company).isEmpty()) {
        args << "--company-name=" + PCM.getItemValueToString(PCE::Company);
    }
    if (!PCM.getItemValueToString(PCE::ProductName).isEmpty()) {
        args << "--product-name=" + PCM.getItemValueToString(
            PCE::ProductName);
    }
    if (!PCM.getItemValueToString(PCE::ProductVersion).isEmpty()) {
        args << "--product-version=" + PCM.getItemValueToString(
            PCE::ProductVersion);
    }
    if (!PCM.getItemValueToString(PCE::FileDescription).isEmpty()) {
        args << "--file-description=" + PCM.getItemValueToString(
            PCE::FileDescription);
    }
    if (!PCM.getItemValueToString(PCE::LegalCopyright).isEmpty()) {
        args << "--copyright=" + PCM.getItemValueToString(
            PCE::LegalCopyright);
    }
    if (!PCM.getItemValueToString(PCE::LegalTrademarks).isEmpty()) {
        args << "--trademarks=" + PCM.getItemValueToString(
            PCE::LegalTrademarks);
    }

    this->packProcess->start(
        PCM.getItemValueToString(PCE::PythonPath), args);

    // console output
    QString outputString = QString("-------------- 开始打包 %1 -------------").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    ui->consoleOutputEdit->appendPlainText(outputString);
    // pack timer
    this->startPackTime = QDateTime::currentDateTime();
    this->packTimer->start(config.getInt(ConfigItem::PackTimerTriggerInterval));
    // console output
    ui->consoleOutputEdit->appendPlainText(
        PCM.getItemValueToString(PCE::PythonPath) + " " + args.
        join(" "));
    Logger::info(
        "开始打包  打包命令: " + QString(
            PCM.getItemValueToString(PCE::PythonPath) + " " + args.
            join(" ")));
}

void MainWindow::stopPack() {
    if (!this->packProcess) {
        ui->consoleOutputEdit->appendPlainText("没有正在执行的打包任务");
        return;
    }
    if (this->packProcess->state() == QProcess::NotRunning) {
        ui->consoleOutputEdit->appendPlainText("打包任务已结束");
        this->packProcess->deleteLater();
        this->packProcess = nullptr;
        return;
    }

    ui->consoleOutputEdit->appendPlainText("正在停止打包任务");
    this->packProcess->terminate();

    QTimer::singleShot(5000, this, [=]() {
        if (this->packProcess->state() != QProcess::NotRunning) {
            ui->consoleOutputEdit->appendPlainText("进程未响应，强制终止中...");
            this->packProcess->kill();
            this->packProcess->deleteLater();
        }
    });
    this->showText("已停止打包任务", 5000, Qt::black, TextPos::SystemMessage, "打包通知");

    this->packTimer->stop();
    ui->startPackBtn->setEnabled(true);
    this->startPackAction->setEnabled(true);
    ui->stopPackBtn->setEnabled(false);
    this->stopPackAction->setEnabled(false);
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
    this->updateUI();
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
    this->updateUI();
    if (!path.isEmpty()) {
        this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
    }
    GDM.setBool(GDIN::isOpenNPF, true);
}

// Slots
void MainWindow::onAddDataFileItemClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件",
                                                    config.getString(ConfigItem::DefaultDataPath));
    if (filePath.isEmpty()) {
        return;
    }
    QStringList dataList = this->dataListModel->stringList();
    dataList << filePath;
    this->dataListModel->setStringList(dataList);
    PCM.appendItemToStringList(PCE::DataList, filePath);
}

void MainWindow::onAddDataDirItemClicked() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                        config.getString(ConfigItem::DefaultDataPath),
                                                        QFileDialog::ShowDirsOnly);
    if (dirPath.isEmpty()) {
        return;
    }

    QStringList dataList = this->dataListModel->stringList();
    dataList << dirPath;
    this->dataListModel->setStringList(dataList);
    PCM.appendItemToStringList(PCE::DataList, dirPath);
}

void MainWindow::onRemoveItemClicked() const {
    QModelIndex index = ui->dataListWidget->currentIndex();
    if (index.isValid() && this->dataListModel) {
        const QString &text = this->dataListModel->stringList().at(index.row());

        this->dataListModel->removeRow(index.row());
        PCM.removeItemFromStringList(PCE::DataList, text);
    }
}

void MainWindow::onFileMenuTriggered(QAction *action) {
    QString text = action->text();
    Logger::info(QString("菜单：文件, 菜单项 %1 触发triggered事件").arg(text));

    if (text == this->controlText->menu_new) {
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
                PCM.setItem(PCE::ProjectPath, dirPath);
                this->genData();
            }
        }

        this->clearText();
    } else if (text == this->controlText->menu_open) {
        this->importProject();
        this->clearText();
    } else if (text == this->controlText->menu_save) {
        this->npfStatusTypeHandler(ProjectConfig::saveProject(GDM.getString(GDIN::npfFilePath),
                                                              config.getBool(ConfigItem::IsSavePackLog)),
                                   GDM.getString(GDIN::npfFilePath));
    } else if (text == this->controlText->menu_saveAs) {
        this->exportProject();
    } else if (text == this->controlText->menu_closeFile) {
        int choose = QMessageBox::question(this, "Nuitka Studio", "关闭后未保存的数据将会丢失，是否确认关闭");
        if (choose == QMessageBox::Yes) {
            PCM.setDefaultValue();
            GDM.setString(GDIN::npfFilePath, "");
            GDM.setBool(GDIN::isOpenNPF, false);
            config.setString(ConfigItem::NpfPath, "");
            this->setWindowTitle("Nuitka Studio");
            this->updateUI();
        }
    }
}

void MainWindow::onHelpMenuTriggered(QAction *action) {
    QString text = action->text();
    Logger::info(QString("菜单：帮助, 菜单项 %1 触发triggered事件").arg(text));

    if (text == this->controlText->menu_help) {
        QDesktopServices::openUrl(QUrl("https://github.com/Redrch/NuitkaStudio"));
    } else if (text == this->controlText->menu_about) {
        AboutWindow *aboutWindow = new AboutWindow(this);
        aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
        aboutWindow->exec();
    }
}

void MainWindow::retranslateCustomUi() const {
    this->controlText->menu_new = tr("新建(&N)");
    this->controlText->menu_open = tr("打开(&O)");
    this->controlText->menu_save = tr("保存(&S)");
    this->controlText->menu_saveAs = tr("另存为(&A)");
    this->controlText->menu_closeFile = tr("关闭文件(&C)");
    this->controlText->menu_help = tr("帮助(&H)");
    this->controlText->menu_about = tr("关于(&A)");

    this->controlText->topbar_pack = tr("打包");
    this->controlText->topbar_settings = tr("设置");
    this->controlText->topbar_packLog = tr("打包日志");

    this->controlText->exit_label = tr("您想要将软件关闭还是最小化至系统托盘");
    this->controlText->exit_trayButton = tr("最小化至系统托盘");
    this->controlText->exit_exitButton = tr("退出软件");
    this->controlText->exit_hideButton = tr("不再显示该窗口（隐藏后行为可以在设置中看到）");

    ui->newAction->setText(this->controlText->menu_new);
    ui->openAction->setText(this->controlText->menu_open);
    ui->saveAction->setText(this->controlText->menu_save);
    ui->saveAsAction->setText(this->controlText->menu_saveAs);
    ui->closeFileAction->setText(this->controlText->menu_closeFile);
    ui->helpAction->setText(this->controlText->menu_help);
    ui->aboutAction->setText(this->controlText->menu_about);

    this->packAction->setText(this->controlText->topbar_pack);
    this->settingsAction->setText(this->controlText->topbar_settings);
    this->packLogAction->setText(this->controlText->topbar_packLog);
}

// Update UI functions
void MainWindow::updateUI() {
    this->updatePackUI();
    this->updateSettingsUI();
    if (!GDM.getString(GDIN::npfFilePath).isEmpty()) {
        this->updatePackLogUI();
    }

    Logger::info("刷新UI");
}

void MainWindow::updatePackUI() const {
    ui->pythonFileEdit->setText(PCM.getItemValueToString(PCE::PythonPath));
    ui->mainPathEdit->setText(PCM.getItemValueToString(PCE::MainfilePath));
    ui->outputPathEdit->setText(PCM.getItemValueToString(PCE::OutputPath));
    ui->outputFileEdit->setText(PCM.getItemValueToString(PCE::OutputFilename));
    ui->iconFileEdit->setText(PCM.getItemValueToString(PCE::IconPath));
    ui->projectPathEdit->setText(PCM.getItemValueToString(PCE::ProjectPath));
    ui->projectNameEdit->setText(PCM.getItemValueToString(PCE::ProjectName));

    ui->standaloneCheckbox->setCheckState(
        PCM.getItemValueToBool(PCE::Standalone)
            ? Qt::CheckState::Checked
            : Qt::CheckState::Unchecked);
    ui->onefileCheckbox->setCheckState(PCM.getItemValueToBool(PCE::Onefile)
                                           ? Qt::CheckState::Checked
                                           : Qt::CheckState::Unchecked);
    ui->removeOutputCheckbox->setCheckState(
        PCM.getItemValueToBool(PCE::RemoveOutput)
            ? Qt::CheckState::Checked
            : Qt::CheckState::Unchecked);
    // LTO
    switch (PCM.getItemValue(PCE::LtoMode).value<LTOMode>()) {
        case LTOMode::Yes:
            ui->ltoYes->setCheckState(Qt::CheckState::Checked);
            break;
        case LTOMode::No:
            ui->ltoNo->setCheckState(Qt::CheckState::Checked);
            break;
        case LTOMode::Auto:
            ui->ltoAuto->setCheckState(Qt::CheckState::Checked);
            break;
    }
    // Data list
    QStringList dataList = PCM.getItemValueToStringList(PCE::DataList);
    this->dataListModel->setStringList(dataList);

    // File info
    ui->fileVersionEdit->setText(PCM.getItemValueToString(PCE::FileVersion));
    ui->companyEdit->setText(PCM.getItemValueToString(PCE::Company));
    ui->productNameEdit->setText(PCM.getItemValueToString(PCE::ProductName));
    ui->productVersionEdit->setText(PCM.getItemValueToString(PCE::ProductVersion));

    ui->fileDescriptitonEdit->setText(
        PCM.getItemValueToString(PCE::FileDescription));
    ui->legalCopyrightEdit->setText(PCM.getItemValueToString(PCE::LegalCopyright));
    ui->legalTrademarksEdit->setText(
        PCM.getItemValueToString(PCE::LegalTrademarks));
}

void MainWindow::updateSettingsUI() const {
    ui->defaultPyPathEdit->setText(config.getString(ConfigItem::DefaultPythonPath));
    ui->defaultMainPathEdit->setText(config.getString(ConfigItem::DefaultMainFilePath));
    ui->defaultOutputPathEdit->setText(config.getString(ConfigItem::DefaultOutputPath));
    ui->defaultIconPathEdit->setText(config.getString(ConfigItem::DefaultIconPath));
    ui->defaultDataPathEdit->setText(config.getString(ConfigItem::DefaultDataPath));

    ui->consoleInputEncodingCombo->setCurrentIndex(
        Utils::enumToInt<EncodingEnum>(config.getEncodingEnum(ConfigItem::ConsoleInputEncoding)));
    ui->consoleOutputEncodingCombo->setCurrentIndex(
        Utils::enumToInt<EncodingEnum>(config.getEncodingEnum(ConfigItem::ConsoleOutputEncoding)));
    ui->languageComboBox->setCurrentIndex(
        Utils::enumToInt<Language>(config.getLanguage(ConfigItem::Language)));

    ui->packTimerTriggerIntervalSpin->setValue(config.getInt(ConfigItem::PackTimerTriggerInterval));
    ui->maxPackLogCountSpin->setValue(config.getInt(ConfigItem::MaxPackLogCount));

    ui->showCloseWindowCheckbox->setChecked(config.getBool(ConfigItem::IsShowCloseWindow));
    ui->hideOnCloseCheckbox->setChecked(config.getBool(ConfigItem::IsHideOnClose));
    if (config.getBool(ConfigItem::IsShowCloseWindow)) {
        ui->hideOnCloseCheckbox->setEnabled(false);
    } else {
        ui->hideOnCloseCheckbox->setEnabled(true);
    }
    ui->splashScreenCheckbox->setChecked(config.getBool(ConfigItem::IsSplashScreen));
    ui->savePackLog->setChecked(config.getBool(ConfigItem::IsSavePackLog));

    ui->tempPathEdit->setText(config.getString(ConfigItem::TempPath));
}

void MainWindow::updatePackLogUI() {
    this->readPackLog();
    QStringList logStringList;
    for (PackLog *log: *this->packLog) {
        logStringList.append(log->logFileName);
    }
    this->packLogModel->setStringList(logStringList);

    if (this->packLog->isEmpty()) {
        ui->packLogContent->clear();
        ui->noteEdit->clear();
        this->currentPackLogIndex = 0;
        return;
    }

    if (this->currentPackLogIndex < 0 || this->currentPackLogIndex >= this->packLog->count()) {
        this->currentPackLogIndex = 0;
    }
    ui->packLogContent->setPlainText(this->packLog->at(0)->logContent);
    ui->noteEdit->setText(this->packLog->at(this->currentPackLogIndex)->logNote);
}

// Connect functions
void MainWindow::connectStackedWidget() {
    connect(this->packAction, &QAction::triggered, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
        this->currentPageIndex = 0;
    });
    connect(this->settingsAction, &QAction::triggered, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
        this->currentPageIndex = 1;
    });
    connect(this->packLogAction, &QAction::triggered, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2);
        this->currentPageIndex = 2;
    });
    connect(this->floatButtonAction, &QAction::triggered, this, [=]() {
        this->hide();
        this->floatButton->show();
    });
}

void MainWindow::connectMenubar() {
    connect(ui->fileMenu, &QMenu::triggered, this, &MainWindow::onFileMenuTriggered);
    connect(ui->helpMenu, &QMenu::triggered, this, &MainWindow::onHelpMenuTriggered);
}

void MainWindow::connectPackPage() {
    // Browse buttons
    // Python file browse button
    connect(ui->pythonFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::PythonPath, QFileDialog::getOpenFileName(
                        this, "Nuitka Studio  Python解释器选择",
                        config.getString(ConfigItem::DefaultPythonPath), "exe(*.exe)"));
        ui->pythonFileEdit->setText(PCM.getItemValueToString(PCE::PythonPath));
    });

    // Main file path browse button
    connect(ui->mainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::MainfilePath, QFileDialog::getOpenFileName(
                        this, "Nuitka Studio  主文件选择",
                        config.getString(ConfigItem::DefaultMainFilePath),
                        "Python file(*.py)"));
        ui->mainPathEdit->setText(PCM.getItemValueToString(PCE::MainfilePath));
    });

    // Output file path browse button
    connect(ui->outputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::OutputPath, QFileDialog::getExistingDirectory(
                        this, "Nuitka Studio  输出路径",
                        config.getString(ConfigItem::DefaultOutputPath),
                        QFileDialog::ShowDirsOnly));
        ui->outputPathEdit->setText(PCM.getItemValueToString(PCE::OutputPath));
    });

    // Project path browse button
    connect(ui->projectPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::ProjectPath, QFileDialog::getExistingDirectory(
                        this, "Nuitka Studio  项目路径",
                        config.getString(ConfigItem::DefaultMainFilePath),
                        QFileDialog::ShowDirsOnly));
        ui->projectPathEdit->setText(
            PCM.getItemValueToString(PCE::ProjectPath));

        PCM.setItem(PCE::ProjectName,
                    PCM.getItemValueToString(
                        PCE::ProjectPath).split("/").last());
        ui->projectNameEdit->setText(
            PCM.getItemValueToString(PCE::ProjectName));
        if (PCM.getItemValueToString(PCE::ProjectPath).isEmpty()) return;

        this->genData();
    });

    // Edits
    // Python file edit
    connect(ui->pythonFileEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::PythonPath, text);
    });

    // Main file path edit
    connect(ui->mainPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::MainfilePath, text);
    });

    // Output file path edit
    connect(ui->outputPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::OutputPath, text);
    });

    // Output file name edit
    connect(ui->outputFileEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::OutputFilename, text);
    });

    // Project path edit
    connect(ui->projectPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::ProjectPath, text);
    });

    // Project name edit
    connect(ui->projectNameEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::ProjectName, text);
    });

    // Icon edit
    connect(ui->iconFileEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::IconPath, text);
    });

    // Build Settings
    // Standalone
    connect(ui->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            PCM.setItem(PCE::Standalone, false);
        } else if (state == Qt::Checked) {
            PCM.setItem(PCE::Standalone, true);
        }
    });
    // Onefile
    connect(ui->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            PCM.setItem(PCE::Onefile, false);
        } else if (state == Qt::Checked) {
            PCM.setItem(PCE::Onefile, true);
        }
    });
    // Remove Output
    connect(ui->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            PCM.setItem(PCE::RemoveOutput, false);
        } else if (state == Qt::Checked) {
            PCM.setItem(PCE::RemoveOutput, true);
        }
    });

    // LTO Mode Checkbox
    // No
    connect(ui->ltoNo, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            PCM.setItem(PCE::LtoMode,
                        QVariant::fromValue<LTOMode>(LTOMode::No));
        }
    });
    // Yes
    connect(ui->ltoYes, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoYes->checkState() == Qt::CheckState::Checked) {
            PCM.setItem(PCE::LtoMode,
                        QVariant::fromValue<LTOMode>(LTOMode::Yes));
        }
    });
    // Auto
    connect(ui->ltoAuto, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoAuto->checkState() == Qt::CheckState::Checked) {
            PCM.setItem(PCE::LtoMode,
                        QVariant::fromValue<LTOMode>(LTOMode::Auto));
        }
    });

    // Modify Data List
    // Add file button
    connect(ui->addFileBtn, &QPushButton::clicked, this, &MainWindow::onAddDataFileItemClicked);
    // Add dir button
    connect(ui->addDirBtn, &QPushButton::clicked, this, &MainWindow::onAddDataDirItemClicked);
    // Remove item button
    connect(ui->removeItemBtn, &QPushButton::clicked, this, &MainWindow::onRemoveItemClicked);

    // Icon browse
    connect(ui->iconFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::IconPath, QFileDialog::getOpenFileName(
                        this, "Nuitka Studio  图标路径", "C:\\",
                        "Icon file(*.jpg *.jpeg *.png *.ico);;All files(*)"));
        ui->iconFileEdit->setText(PCM.getItemValueToString(PCE::IconPath));
    });

    // Start pack
    connect(ui->startPackBtn, &QPushButton::clicked, this, &MainWindow::startPack);
    // Stop pack
    connect(ui->stopPackBtn, &QPushButton::clicked, this, &MainWindow::stopPack);
    // Clear Console Edit
    connect(ui->clearConsoleBtn, &QPushButton::clicked, this, [=]() {
        ui->consoleOutputEdit->clear();
    });
    // Import button
    connect(ui->importBtn, &QPushButton::clicked, this, &MainWindow::importProject);

    // Pack Timer
    connect(this->packTimer, &QTimer::timeout, this, [=]() {
        auto now = QDateTime::currentDateTime();
        qint64 time = now.toMSecsSinceEpoch() - this->startPackTime.toMSecsSinceEpoch();
        QString timeString = Utils::formatMilliseconds(time);
        this->messageLabel->setText(timeString);
    });

    // Gen paths button
    connect(ui->genPathsButton, &QPushButton::clicked, this, [=] {
        this->genData();
    });

    // file info
    connect(ui->fileVersionEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::FileVersion, text);
        this->genFileInfo();
        this->updateUI();
    });
    connect(ui->companyEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::Company, text);
    });
    connect(ui->productNameEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::ProductName, text);
    });
    connect(ui->productVersionEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::ProductVersion, text);
    });
    connect(ui->fileDescriptitonEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::FileDescription, text);
    });
    connect(ui->legalCopyrightEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::LegalCopyright, text);
    });
    connect(ui->legalTrademarksEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PCM.setItem(PCE::LegalTrademarks, text);
    });
}

void MainWindow::connectSettingsPage() {
    // General Settings
    // Console Input Encoding
    connect(ui->consoleInputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = Utils::intToEnum<EncodingEnum>(index);
                config.setEncodingEnum(ConfigItem::ConsoleInputEncoding, encoding);
            });
    // Console Output Encoding
    connect(ui->consoleOutputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = Utils::intToEnum<EncodingEnum>(index);
                config.setEncodingEnum(ConfigItem::ConsoleOutputEncoding, encoding);
            });
    // Language
    connect(ui->languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                Language language = Utils::intToEnum<Language>(index);
                config.setLanguage(ConfigItem::Language, language);
                if (GDM.get(GDIN::translator).value<QTranslator *>()->load(
                    QString(":/lang/%1.qm").arg(Utils::enumToString(language)))) {
                    QApplication::installTranslator(GDM.get(GDIN::translator).value<QTranslator *>());
                    emit eventBus.languageChanged(language);
                } else {
                    Logger::error("无法加载语言文件");
                }
            });
    // Pack Timer Trigger Interval
    connect(ui->packTimerTriggerIntervalSpin, QOverload<int>::of(&ElaSpinBox::valueChanged), this, [=](int value) {
        config.set(ConfigItem::PackTimerTriggerInterval, value);
    });
    // Max Pack Log Count
    connect(ui->maxPackLogCountSpin, QOverload<int>::of(&ElaSpinBox::valueChanged), this, [=](int value) {
        config.set(ConfigItem::MaxPackLogCount, value);
    });
    // Is Show Close Window
    connect(ui->showCloseWindowCheckbox, QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            config.setBool(ConfigItem::IsShowCloseWindow, true);
        } else {
            config.setBool(ConfigItem::IsShowCloseWindow, false);
        }
        // Is Enabled hideOnCloseCheckbox
        if (config.getBool(ConfigItem::IsShowCloseWindow)) {
            ui->hideOnCloseCheckbox->setEnabled(false);
        } else {
            ui->hideOnCloseCheckbox->setEnabled(true);
        }
    });
    // Is Hide On Close
    connect(ui->hideOnCloseCheckbox, QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            config.setBool(ConfigItem::IsHideOnClose, true);
        } else {
            config.setBool(ConfigItem::IsHideOnClose, false);
        }
    });
    // Is Splash Screen
    connect(ui->splashScreenCheckbox, QCheckBox::toggled, this, [=](bool checked) {
        config.setBool(ConfigItem::IsSplashScreen, checked);
    });
    // Is Save Pack log
    connect(ui->savePackLog, &QCheckBox::toggled, this, [=](bool checked) {
        config.setBool(ConfigItem::IsSavePackLog, checked);
    });
    // Temp Path
    connect(ui->tempPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.set(ConfigItem::TempPath, text);
    });


    // Default Path Settings
    // Browse Buttons
    // Default Python Path Browse
    connect(ui->defaultPyPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.set(ConfigItem::DefaultPythonPath,
                   QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认Python解释器路径选择",
                                                     config.getString(ConfigItem::DefaultPythonPath),
                                                     QFileDialog::ShowDirsOnly));
        ui->defaultPyPathEdit->setText(config.getString(ConfigItem::DefaultPythonPath));
    });
    // Default Main File Path Browse
    connect(ui->defaultMainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.set(ConfigItem::DefaultMainFilePath,
                   QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认主文件路径选择",
                                                     config.getString(ConfigItem::DefaultMainFilePath),
                                                     QFileDialog::ShowDirsOnly));
        ui->defaultMainPathEdit->setText(config.getString(ConfigItem::DefaultMainFilePath));
    });
    // Default Output Path Browse
    connect(ui->defaultOutputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.set(ConfigItem::DefaultOutputPath,
                   QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认输出路径选择",
                                                     config.getString(ConfigItem::DefaultOutputPath),
                                                     QFileDialog::ShowDirsOnly));
        ui->defaultOutputPathEdit->setText(config.getString(ConfigItem::DefaultOutputPath));
    });
    // Default Icon Path Browse
    connect(ui->defaultIconPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.set(ConfigItem::DefaultIconPath,
                   QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认图标路径选择",
                                                     config.getString(ConfigItem::DefaultIconPath),
                                                     QFileDialog::ShowDirsOnly));
        ui->defaultIconPathEdit->setText(config.getString(ConfigItem::DefaultIconPath));
    });
    // Default Data Path Browse
    connect(ui->defaultDataPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.set(ConfigItem::DefaultDataPath,
                   QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认数据路径选择",
                                                     config.getString(ConfigItem::DefaultDataPath),
                                                     QFileDialog::ShowDirsOnly));
        ui->defaultDataPathEdit->setText(config.getString(ConfigItem::DefaultDataPath));
    });

    // Line Edits
    // Python Edit
    connect(ui->defaultPyPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setString(ConfigItem::DefaultPythonPath, text);
    });
    // Main file Edit
    connect(ui->defaultMainPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setString(ConfigItem::DefaultMainFilePath, text);
    });
    // Output Edit
    connect(ui->defaultOutputPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setString(ConfigItem::DefaultOutputPath, text);
    });
    // Icon Edit
    connect(ui->defaultIconPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setString(ConfigItem::DefaultIconPath, text);
    });
    // Data Edit
    connect(ui->defaultDataPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setString(ConfigItem::DefaultDataPath, text);
    });

    // Save button
    connect(ui->saveSettingsBtn, &QPushButton::clicked, this, [=]() {
        config.writeConfig();
    });
}

void MainWindow::connectTrayMenu() {
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
}

void MainWindow::connectOther() {
    connect(&GDM, &GlobalData::valueChanged, this, [=](const QString &valueName, const QVariant &newValue) {
        if (valueName == GDIN::isOpenNPF) {
            if (newValue.toBool()) {
                this->enabledInput();
            } else {
                this->noEnableInput();
            }
        }
    });
    // 手动变色
    connect(ElaTheme::getInstance(), &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode mode) {
        const QColor &textColor = ElaThemeColor(mode, ThemeColor::BasicText);
        QString textColorHex = textColor.name();
        QString textStyleSheet = QString("color: %1;").arg(textColorHex);

        ui->standaloneCheckbox->setStyleSheet(ui->standaloneCheckbox->styleSheet() += textStyleSheet);
        ui->onefileCheckbox->setStyleSheet(ui->onefileCheckbox->styleSheet() += textStyleSheet);
        ui->removeOutputCheckbox->setStyleSheet(ui->removeOutputCheckbox->styleSheet() += textStyleSheet);

        ui->ltoNo->setStyleSheet(ui->ltoNo->styleSheet() += textStyleSheet);
        ui->ltoYes->setStyleSheet(ui->ltoYes->styleSheet() += textStyleSheet);
        ui->ltoAuto->setStyleSheet(ui->ltoAuto->styleSheet() += textStyleSheet);

        ui->showCloseWindowCheckbox->setStyleSheet(ui->showCloseWindowCheckbox->styleSheet() += textStyleSheet);
        ui->hideOnCloseCheckbox->setStyleSheet(ui->hideOnCloseCheckbox->styleSheet() += textStyleSheet);
        ui->splashScreenCheckbox->setStyleSheet(ui->splashScreenCheckbox->styleSheet() += textStyleSheet);
        ui->savePackLog->setStyleSheet(ui->savePackLog->styleSheet() += textStyleSheet);
    });

    connect(this->floatButton, &FloatButton::startPack, this, &MainWindow::startPack);
    connect(this->floatButton, &FloatButton::stopPack, this, &MainWindow::stopPack);
    connect(this->floatButton, &FloatButton::showMainWindow, this, [=]() {
        this->floatButton->hide();
        this->showNormal();
        this->activateWindow();
    });

    connect(&eventBus, &EventBus::languageChanged, [=]() {
        ui->retranslateUi(this);
        this->retranslateCustomUi();
        this->updateUI();
    });
}

void MainWindow::connectPackLog() {
    connect(ui->packLogFileList, &ElaListView::clicked, this, [=](const QModelIndex &index) {
        int row = index.row();
        this->currentPackLogIndex = row;
        PackLog *packLog = this->packLog->at(row);
        ui->packLogContent->setPlainText(packLog->logContent);
        ui->noteEdit->setText(packLog->logNote);
    });

    connect(ui->noteEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        PackLog *packLog = this->packLog->at(this->currentPackLogIndex);
        packLog->logNote = text;
        this->noteObject.insert(packLog->logFileName, text);
        this->saveNote();
    });
}

// Init functions
void MainWindow::initUI() {
    // Pack log
    ui->packLogFileList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Status bar
    this->messageLabel = new ElaText("", 9, this);
    this->messageLabel->setAlignment(Qt::AlignCenter);
    ui->statusbar->addWidget(this->messageLabel);
    ui->statusbar->addPermanentWidget(this->messageLabel, 1);

    this->trayIcon = new QSystemTrayIcon(QIcon(":/logo"), this);
    this->trayIcon->setToolTip("Nuitka Studio");

    // Menubar
    // file menu
    ui->newAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_N));
    ui->openAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_O));
    ui->saveAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_S));
    ui->saveAsAction->setShortcut(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_S));
    // tool menu
    ui->packLogAction->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_P));

    // Tray menu
    this->trayMenu = new QMenu(this);
    this->startPackAction = new QAction("开始打包项目", this);
    this->stopPackAction = new QAction("停止打包项目", this);
    this->showAction = new QAction("显示", this);
    this->quitAction = new QAction("退出", this);
    this->stopPackAction->setEnabled(false);

    trayMenu->addAction(startPackAction);
    trayMenu->addAction(stopPackAction);
    trayMenu->addAction(showAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    trayIcon->show();

    // Settings Page
    ui->packTimerTriggerIntervalSpin->setButtonMode(ElaSpinBoxType::Compact);
    ui->maxPackLogCountSpin->setButtonMode(ElaSpinBoxType::Compact);

    // init top text label
    this->topTextLabel = new QLabel("", this);

    // set models
    ui->dataListWidget->setModel(this->dataListModel);
    ui->packLogFileList->setModel(packLogModel);

    // lock pack ui
    if (!GDM.getBool(GDIN::isOpenNPF)) {
        this->noEnableInput();
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

    this->initMenuBar();
}

void MainWindow::initMenuBar() {
    this->menuBar = new ElaMenuBar(this);
    menuBar->setFixedHeight(30);
    QWidget *customWidget = new QWidget(this);
    customWidget->setFixedWidth(300);
    QVBoxLayout *customLayout = new QVBoxLayout(customWidget);
    customLayout->setContentsMargins(0, 0, 0, 0);

    customLayout->addWidget(this->menuBar);
    customLayout->addStretch();
    this->setCustomWidget(ElaAppBarType::MiddleArea, customWidget);

    this->packAction = this->menuBar->addElaIconAction(ElaIconType::BoxesPacking, this->controlText->topbar_pack);
    this->settingsAction = this->menuBar->addElaIconAction(ElaIconType::Gear, this->controlText->topbar_settings);
    this->packLogAction = this->menuBar->addElaIconAction(ElaIconType::File, this->controlText->topbar_packLog);
    this->floatButtonAction = this->menuBar->addElaIconAction(ElaIconType::SquareXmark, "");
}

// gen path functions
void MainWindow::genData(bool isUpdateUI) {
    if (PCM.getItemValueToString(PCE::ProjectPath).isEmpty()) {
        QMessageBox::warning(this, "Nuitka Studio Warning", "请填写项目路径");
        return;
    }
    if (PCM.getItemValueToString(PCE::ProjectName).isEmpty()) {
        PCM.setItem(PCE::ProjectName,
                    PCM.getItemValueToString(
                        PCE::ProjectPath).split("/").last());
        if (PCM.getItemValueToString(PCE::ProjectName).isEmpty()) {
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
        this->updateUI();
    }
    Logger::info("生成数据");
}

void MainWindow::genPythonPath() {
    QDir projectDir(PCM.getItemValueToString(PCE::ProjectPath));
    if (projectDir.exists(
        PCM.getItemValueToString(PCE::ProjectPath) + "/.venv")) {
        PCM.setItem(PCE::PythonPath,
                    PCM.getItemValueToString(
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
                    PCM.setItem(PCE::PythonPath, fi.filePath());
                }
            }
        }
    }
}

void MainWindow::genMainfilePath() {
    QDir projectDir(PCM.getItemValueToString(PCE::ProjectPath));
    if (projectDir.exists(PCM.getItemValueToString(PCE::ProjectPath) + "/src")
        || projectDir.exists(
            PCM.getItemValueToString(PCE::ProjectPath) + "/source")) {
        PCM.setItem(PCE::MainfilePath,
                    PCM.getItemValueToString(
                        PCE::ProjectPath) + "/src/main.py");
    } else {
        PCM.setItem(PCE::MainfilePath,
                    PCM.getItemValueToString(
                        PCE::ProjectPath) +
                    "/main.py");
    }
}

void MainWindow::genOutputPath() {
    PCM.setItem(PCE::OutputPath,
                PCM.getItemValueToString(
                    PCE::ProjectPath) +
                "/output");
}

void MainWindow::genOutputName() {
    PCM.setItem(PCE::OutputFilename,
                PCM.getItemValueToString(
                    PCE::ProjectName) +
                ".exe");
}

void MainWindow::genFileInfo() {
    PCM.setItem(PCE::ProductName,
                PCM.getItemValueToString(
                    PCE::ProjectName));
}

// events
void MainWindow::closeEvent(QCloseEvent *event) {
    if (config.getBool(ConfigItem::IsShowCloseWindow)) {
        QDialog dialog(this);
        QVBoxLayout mainLayout;
        dialog.setLayout(&mainLayout);
        // label
        QLabel label(this->controlText->exit_label);
        label.setAlignment(Qt::AlignCenter);
        mainLayout.addWidget(&label);
        // buttons
        QPushButton trayBtn(this->controlText->exit_trayButton);
        mainLayout.addWidget(&trayBtn);
        QPushButton exitBtn(this->controlText->exit_exitButton);
        mainLayout.addWidget(&exitBtn);
        // hide
        QCheckBox hideCheckbox(this->controlText->exit_hideButton);
        mainLayout.addWidget(&hideCheckbox);

        bool shouldHide = false;
        bool shouldQuit = false;
        // connect
        connect(&trayBtn, &QPushButton::clicked, [&] {
            shouldHide = true;
            dialog.accept();
        });
        connect(&exitBtn, &QPushButton::clicked, [&] {
            shouldQuit = true;
            dialog.accept();
        });
        connect(&hideCheckbox, &QCheckBox::stateChanged, [=](int state) {
            if (state == Qt::Checked) {
                config.setBool(ConfigItem::IsShowCloseWindow, false);
            } else if (state == Qt::Unchecked) {
                config.setBool(ConfigItem::IsShowCloseWindow, true);
            }
            config.writeConfig();
        });

        dialog.exec();

        if (shouldQuit) {
            config.setBool(ConfigItem::IsHideOnClose, false);
            event->accept();
            config.writeConfig();
            qApp->quit();
        } else if (shouldHide) {
            config.setBool(ConfigItem::IsHideOnClose, true);
            this->hide();
            event->ignore();
            config.writeConfig();
        } else {
            event->ignore();
        }
    } else {
        if (config.getBool(ConfigItem::IsHideOnClose)) {
            this->hide();
            event->ignore();
        } else {
            event->accept();
            qApp->quit();
        }
    }
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
                        this->enabledInput();
                        this->setWindowTitle(pathInfo.fileName() + " - Nuitka Studio");
                    }
                } else if (suffix == "py") {
                    if (filePath.split("/").contains("src") || filePath.split("/").contains("source") ||
                        pathInfo.fileName() == "main.py") {
                        QString projectPath = pathInfo.absolutePath();
                        PCM.setItem(PCE::ProjectPath, projectPath);
                        this->genData();
                        this->clearText(TextPos::TopLabel);
                        this->enabledInput();
                    }
                }
            }
        }
    }
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Tab) {
            // 按Tab键切换页面
            switch (this->currentPageIndex) {
                case 0:
                    ui->stackedWidget->setCurrentIndex(1);
                    this->currentPageIndex = 1;
                    break;
                case 1:
                    ui->stackedWidget->setCurrentIndex(2);
                    this->currentPageIndex = 2;
                    break;
                case 2:
                    ui->stackedWidget->setCurrentIndex(0);
                    this->currentPageIndex = 0;
                    break;
                default:
                    break;
            }

            return true;
        }
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

void MainWindow::noEnableInput() const {
    ui->projectPathEdit->setEnabled(false);
    ui->projectPathBrowseBtn->setEnabled(false);
    ui->projectNameEdit->setEnabled(false);
    ui->mainPathEdit->setEnabled(false);
    ui->mainPathBrowseBtn->setEnabled(false);
    ui->pythonFileEdit->setEnabled(false);
    ui->pythonFileBrowseBtn->setEnabled(false);
    ui->outputPathEdit->setEnabled(false);
    ui->outputPathBrowseBtn->setEnabled(false);
    ui->outputFileEdit->setEnabled(false);
    ui->genPathsButton->setEnabled(false);

    ui->standaloneCheckbox->setEnabled(false);
    ui->onefileCheckbox->setEnabled(false);
    ui->removeOutputCheckbox->setEnabled(false);
    ui->ltoAuto->setEnabled(false);
    ui->ltoYes->setEnabled(false);
    ui->ltoNo->setEnabled(false);

    ui->dataListWidget->setEnabled(false);
    ui->addFileBtn->setEnabled(false);
    ui->addDirBtn->setEnabled(false);
    ui->removeItemBtn->setEnabled(false);
    ui->iconFileEdit->setEnabled(false);
    ui->iconFileBrowseBtn->setEnabled(false);

    ui->fileVersionEdit->setEnabled(false);
    ui->companyEdit->setEnabled(false);
    ui->productNameEdit->setEnabled(false);
    ui->productVersionEdit->setEnabled(false);
    ui->legalCopyrightEdit->setEnabled(false);
    ui->legalTrademarksEdit->setEnabled(false);
    ui->fileDescriptitonEdit->setEnabled(false);

    this->startPackAction->setEnabled(false);
    this->stopPackAction->setEnabled(false);
}

void MainWindow::enabledInput() const {
    ui->projectPathEdit->setEnabled(true);
    ui->projectPathBrowseBtn->setEnabled(true);
    ui->projectNameEdit->setEnabled(true);
    ui->mainPathEdit->setEnabled(true);
    ui->mainPathBrowseBtn->setEnabled(true);
    ui->pythonFileEdit->setEnabled(true);
    ui->pythonFileBrowseBtn->setEnabled(true);
    ui->outputPathEdit->setEnabled(true);
    ui->outputPathBrowseBtn->setEnabled(true);
    ui->outputFileEdit->setEnabled(true);
    ui->genPathsButton->setEnabled(true);

    ui->standaloneCheckbox->setEnabled(true);
    ui->onefileCheckbox->setEnabled(true);
    ui->removeOutputCheckbox->setEnabled(true);
    ui->ltoAuto->setEnabled(true);
    ui->ltoYes->setEnabled(true);
    ui->ltoNo->setEnabled(true);

    ui->dataListWidget->setEnabled(true);
    ui->addFileBtn->setEnabled(true);
    ui->addDirBtn->setEnabled(true);
    ui->removeItemBtn->setEnabled(true);
    ui->iconFileEdit->setEnabled(true);
    ui->iconFileBrowseBtn->setEnabled(true);

    ui->fileVersionEdit->setEnabled(true);
    ui->companyEdit->setEnabled(true);
    ui->productNameEdit->setEnabled(true);
    ui->productVersionEdit->setEnabled(true);
    ui->legalCopyrightEdit->setEnabled(true);
    ui->legalTrademarksEdit->setEnabled(true);
    ui->fileDescriptitonEdit->setEnabled(true);

    this->startPackAction->setEnabled(true);
    this->stopPackAction->setEnabled(true);
}

// util functions
void MainWindow::readPackLog() {
    this->packLog->clear();
    if (GDM.getString(GDIN::npfFilePath).isEmpty()) {
        Logger::warn("NPF文件路径为空，无法调用MainWindow::readPackLog函数");
        return;
    }
    QString packLogRoot = GDM.getString(GDIN::packLogPath) + "/" + QFileInfo(GDM.getString(GDIN::npfFilePath)).
                          fileName();

    if (!QDir(packLogRoot).exists()) {
        Logger::warn("MainWindow::readPackLog(): 打包日志目录不存在, " + packLogRoot);
        return;
    }
    QStringList packLogFileList = QDir(packLogRoot).entryList(QDir::Files);
    if (packLogFileList.contains("note.json")) {
        packLogFileList.removeOne("note.json");
    }

    for (const QString &packLogFile: packLogFileList) {
        QFile file(packLogRoot + "/" + packLogFile);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, "Nuitka Studio Error", "无法加载打包日志");
            return;
        }

        QString packLog = QString::fromUtf8(file.readAll());
        QString note = this->noteObject.value(packLogFile).toString();

        this->packLog->append(new PackLog(packLogFile, packLog, note));
    }
}

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

void MainWindow::saveNote() const {
    this->noteFile->open(QIODevice::WriteOnly);
    this->noteFile->write(QJsonDocument(this->noteObject).toJson());
    this->noteFile->close();
}
