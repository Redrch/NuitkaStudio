//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->projectConfig = new ProjectConfig(this);

    if (!QFile::exists(config.getConfigPath())) {
        config.writeConfig();
    }
    config.readConfig();

    this->packTimer = new QTimer(this);

    // Create the temp path
    if (!QDir(config.getConfigToString(SettingsEnum::TempPath)).exists()) {
        if (!QDir().mkpath(config.getConfigToString(SettingsEnum::TempPath))) {
            Logger::warn("缓存文件夹创建失败");
        }
    }

    // Init UI
    this->initExportPage();
    this->initStatusBar();
    this->initTrayMenu();

    // Connect signal and slot
    this->connectStackedWidget();
    this->connectMenubar();
    this->connectPackPage();
    this->connectSettingsPage();
    this->connectExportPage();
    this->connectTrayMenu();

    if (!config.getConfigToString(SettingsEnum::NpfPath).isEmpty()) {
        QString filePath = this->projectConfig->importProject(config.getConfigToString(SettingsEnum::NpfPath));
        if (!GDM.getString(NPF_FILE_PATH).isEmpty()) {
            this->setWindowTitle(filePath.split("/").last() + " - Nuitka Studio");
        }
    }

    this->updateUI();

    Logger::info("初始化MainWindow类完成");
}

MainWindow::~MainWindow() {
    delete this->projectConfig;
    delete ui;
}

void MainWindow::startPack() {
    ui->startPackBtn->setEnabled(false);
    ui->stopPackBtn->setEnabled(true);
    QElapsedTimer timer;

    QDateTime now = QDateTime::currentDateTime();
    QString nowString = now.toString("yyyy-MM-dd_HH-mm-ss");
    QString zipLogPath = NPF_PACK_LOG_PATH + "/" + nowString + ".log";
    QString logPath = config.getConfigToString(SettingsEnum::TempPath) + "/pack_log/" + nowString + ".log";
    QString logDir = config.getConfigToString(SettingsEnum::TempPath) + "/pack_log";
    if (!QDir(logDir).exists()) {
        if (!QDir().mkpath(logDir)) {
            Logger::warn("打包日志缓存文件夹创建失败");
        }
    }
    auto *logFile = new QFile(logPath);
    logFile->open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

    timer.start();

    this->packProcess = new QProcess(this);

    // Signals and slots
    // 合并普通和错误输出
    this->packProcess->setProcessChannelMode(QProcess::MergedChannels);
    // output
    connect(this->packProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString out = QString::fromLocal8Bit(this->packProcess->readAllStandardOutput());
        ui->consoleOutputEdit->append(out);
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
                ui->consoleOutputEdit->append(endOutString);
                Logger::info(QString("----------- 打包结束 耗时: %1 ----------").arg(timeString));
                this->packProcess->deleteLater();
                this->packTimer->stop();
                ui->startPackBtn->setEnabled(true);
                ui->stopPackBtn->setEnabled(false);

                if (logFile) {
                    logFile->flush();
                    logFile->close();
                    logFile->deleteLater();
                }

                QString tempExtractPath = config.getConfigToString(SettingsEnum::TempPath) + "/npf_repack_" + nowString;

                if (!QDir().mkpath(tempExtractPath)) {
                    Logger::error("创建解压文件夹失败，已停止归档");
                    return;
                }

                QStringList extractedFiles = JlCompress::extractDir(GDM.getString(NPF_FILE_PATH), tempExtractPath);
                if (extractedFiles.isEmpty()) {
                    ui->consoleOutputEdit->append("警告：原 NPF 文件内容解压失败或为空！归档已取消以保护原文件。");
                    return; // 防止覆盖原包
                }

                if (!QFile::exists(tempExtractPath + "/data.json")) {
                    ui->consoleOutputEdit->append("致命错误：解压目录中未找到 data.json，停止打包。");
                    return;
                }

                QString newLogTargetDir = tempExtractPath + "/pack_log";
                if (!QDir().mkpath(newLogTargetDir)) {
                    Logger::error("创建缓存日志文件失败，已停止归档");
                    return;
                }
                QString newLogFilePath = newLogTargetDir + "/" + nowString + ".log";
                QFile::copy(logPath, newLogFilePath);

                QStringList fileList = QDir(newLogTargetDir).entryList(QDir::Files, QDir::LocaleAware | QDir::Name);
                while (fileList.count() > config.getConfigToInt(SettingsEnum::MaxPackLogCount)) {
                    fileList.removeFirst();
                    QFile::remove(newLogTargetDir + "/" + fileList[0]);
                }

                if (Compress::compressDir(tempExtractPath, GDM.getString(NPF_FILE_PATH))) {
                    ui->consoleOutputEdit->append("日志归档成功！");
                } else {
                    ui->consoleOutputEdit->append("归档失败：无法重写 NPF 文件。");
                }

                QDir(tempExtractPath).removeRecursively();

                ui->consoleOutputEdit->append(QString("打包日志已存储至<npf_root>/pack_log/" + nowString + ".log"));
                Logger::info(QString("打包日志已存储至<npf_root>/pack_log/" + nowString + ".log"));
            });
    // error occurred
    connect(this->packProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        qWarning() << "command error: " << error;
        ui->consoleOutputEdit->append("Error: " + Utils::processErrorToString(error));
        logFile->write(QString("Error: " + Utils::processErrorToString(error)).toUtf8());
        Logger::error("Error: " + Utils::processErrorToString(error));
    });

    if (PCM.getItemValueToString(PCE::PythonPath).isEmpty()) {
        ui->consoleOutputEdit->append("python解释器路径为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (PCM.getItemValueToString(PCE::MainfilePath).isEmpty()) {
        ui->consoleOutputEdit->append("主文件路径为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (PCM.getItemValueToString(PCE::OutputPath).isEmpty()) {
        ui->consoleOutputEdit->append("输出目录为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (PCM.getItemValueToString(PCE::OutputFilename).isEmpty()) {
        ui->consoleOutputEdit->append("输出文件名为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->startPackBtn->setEnabled(false);
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
    ui->consoleOutputEdit->append(outputString);
    // pack timer
    this->startPackTime = QDateTime::currentDateTime();
    this->packTimer->start(config.getConfigToInt(SettingsEnum::PackTimerTriggerInterval));
    // console output
    ui->consoleOutputEdit->append(
        PCM.getItemValueToString(PCE::PythonPath) + " " + args.
        join(" "));
    Logger::info(
        "开始打包  打包命令: " + QString(
            PCM.getItemValueToString(PCE::PythonPath) + " " + args.
            join(" ")));
}

void MainWindow::stopPack() {
    if (!this->packProcess) {
        ui->consoleOutputEdit->append("没有正在执行的打包任务");
        return;
    }
    if (this->packProcess->state() == QProcess::NotRunning) {
        ui->consoleOutputEdit->append("打包任务已结束");
        this->packProcess = nullptr;
        return;
    }

    ui->consoleOutputEdit->append("正在尝试停止打包任务");
    this->packProcess->terminate();

    QTimer::singleShot(5000, this, [=]() {
        if (this->packProcess->state() != QProcess::NotRunning) {
            ui->consoleOutputEdit->append("进程未响应，强制终止中...");
            this->packProcess->kill();
        }
    });

    this->packTimer->stop();
    ui->startPackBtn->setEnabled(true);
    ui->stopPackBtn->setEnabled(false);
}

void MainWindow::importProject() {
    QString filePath = this->projectConfig->importProject();
    // Update UI
    this->updateUI();
    if (!GDM.getString(NPF_FILE_PATH).isEmpty()) {
        this->setWindowTitle(filePath.split("/").last() + " - Nuitka Studio");
    }
}

void MainWindow::exportProject() {
    QString filePath = this->projectConfig->exportProject();
    this->updateUI();
    if (!GDM.getString(NPF_FILE_PATH).isEmpty()) {
        this->setWindowTitle(filePath.split("/").last() + " - Nuitka Studio");
    }
}

void MainWindow::newProject() {
    auto *newProjectWindow = new NewProjectWindow(this);
    auto *process = new QProcess(this);
    newProjectWindow->setWindowFlags(newProjectWindow->windowFlags() | Qt::Window);
    newProjectWindow->exec();
    this->messageLabel->setText(
        QString("正在为项目%1安装nuitka...").arg(
            PCM.getItemValueToString(PCE::ProjectName)));
    newProjectWindow->installNuitka(process);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                this->messageLabel->setText(QString("项目%1的nuitka安装完毕").arg(
                    PCM.getItemValueToString(PCE::ProjectName)));
                QTimer::singleShot(3000, this, [=]() {
                    this->messageLabel->clear();
                });
                newProjectWindow->deleteLater();
            });
    this->updateUI();
    this->genFileInfo();
}

// Slots
void MainWindow::onAddDataFileItemClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件",
                                                    config.getConfigToString(SettingsEnum::DefaultDataPath));
    if (filePath == "") {
        return;
    }
    ui->dataListWidget->addItem(filePath);
    PCM.appendItemToStringList(PCE::DataList, filePath);
}

void MainWindow::onAddDataDirItemClicked() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                        config.getConfigToString(SettingsEnum::DefaultDataPath),
                                                        QFileDialog::ShowDirsOnly);
    if (dirPath == "") {
        return;
    }

    ui->dataListWidget->addItem(dirPath);
    PCM.appendItemToStringList(PCE::DataList, dirPath);
}

void MainWindow::onRemoveItemClicked() {
    QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
    if (removeItem == nullptr) {
        return;
    }

    PCM.removeItemFromStringList(PCE::DataList, removeItem->text());
    delete removeItem;
}

void MainWindow::onProjectTableCellDoubleClicked(int row, int column) {
    // Data List
    if (row == 8 and column == 1) {
        auto *dataListWindow = new ExportDataListWindow(this);
        dataListWindow->setWindowFlags(dataListWindow->windowFlags() | Qt::Window);
        dataListWindow->setAttribute(Qt::WA_DeleteOnClose);

        dataListWindow->updateUI();
        dataListWindow->show();
    }
}

void MainWindow::onFileMenuTriggered(QAction *action) {
    QString text = action->text();
    Logger::info(QString("菜单：文件, 菜单项 %1 触发triggered事件").arg(text));

    if (text == "新建(&N)") {
        this->newProject();
    } else if (text == "导入(&I)") {
        this->importProject();
    } else if (text == "导出(&E)") {
        this->exportProject();
    }
}

void MainWindow::onToolMenuTriggered(QAction *action) {
    QString text = action->text();
    Logger::info(QString("菜单：工具, 菜单项 %1 触发triggered事件").arg(text));

    if (text == "打包日志(&P)") {
        if (GDM.getString(NPF_FILE_PATH).isEmpty()) {
            QMessageBox::critical(this, "Nuitka Studio Error", "请先指定NPF文件再使用此功能");
            return;
        }

        // read the pack log
        QString nowString = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
        QString zipTempPath = config.getConfigToString(SettingsEnum::TempPath) + "/npf_repack_" + nowString;
        QString packLogPath = zipTempPath + "/pack_log";
        Compress::extractZip(GDM.getString(NPF_FILE_PATH), zipTempPath);
        QStringList logList = QDir(packLogPath).entryList(QDir::Files | QDir::NoDotAndDotDot);
        QMap<QString, QString> logMap;
        for (const QString &logFileName: logList) {
            QFile file(packLogPath + "/" + logFileName);
            if (!file.open(QIODevice::ReadOnly)) {
                QMessageBox::warning(this, "Nuitka Studio Warning", "无法打开打包日志文件" + logFileName);
                Logger::warn("无法打开打包日志文件" + logFileName);
                continue;
            }
            QString log = QString::fromUtf8(file.readAll());
            logMap.insert(logFileName, log);
        }
        // remove temp files
        QDir(zipTempPath).removeRecursively();
        // show the pack log window
        PackLogWindow *logWindow = new PackLogWindow;
        logWindow->setWindowFlags(logWindow->windowFlags() | Qt::Window);
        logWindow->setLog(logMap);
        logWindow->show();
    }
}

void MainWindow::onHelpMenuTriggered(QAction *action) {
    QString text = action->text();
    Logger::info(QString("菜单：帮助, 菜单项 %1 触发triggered事件").arg(text));

    if (text == "帮助(&H)") {
        QDesktopServices::openUrl(QUrl("https://github.com/Redrch/NuitkaStudio"));
    } else if (text == "关于(&A)") {
        auto *aboutWindow = new AboutWindow(this);
        aboutWindow->setWindowFlags(aboutWindow->windowFlags() | Qt::Window);
        aboutWindow->setAttribute(Qt::WA_DeleteOnClose);
        aboutWindow->move(100, 100);
        aboutWindow->show();
    }
}

// Update UI functions
void MainWindow::updateUI() const {
    this->updateExportTable();
    this->updatePackUI();
    this->updateSettingsUI();

    Logger::info("刷新UI");
}

void MainWindow::updateExportTable() const {
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::PythonPath)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::PythonPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::MainfilePath)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::MainfilePath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::OutputPath)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::OutputPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::OutputFilename)), 1, new QTableWidgetItem(
            PCM.getItemValueToString(PCE::OutputFilename)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::IconPath)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::IconPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::ProjectPath)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::ProjectPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::ProjectName)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::ProjectName)));

    // file info
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::FileVersion)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::FileVersion)));

    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(PCE::Company)), 1,
        new QTableWidgetItem(PCM.getItemValueToString(PCE::Company)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(PCE::ProductName)), 1,
                              new QTableWidgetItem(
                                  PCM.getItemValueToString(PCE::ProductName)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(PCE::ProductVersion)), 1,
                              new QTableWidgetItem(
                                  PCM.getItemValueToString(PCE::ProductVersion)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(PCE::FileDescription)), 1,
                              new QTableWidgetItem(
                                  PCM.getItemValueToString(PCE::FileDescription)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(PCE::LegalCopyright)), 1,
                              new QTableWidgetItem(
                                  PCM.getItemValueToString(PCE::LegalCopyright)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(PCE::LegalTrademarks)), 1,
                              new QTableWidgetItem(
                                  PCM.getItemValueToString(PCE::LegalTrademarks)));


    if (this->standaloneCheckbox)
        this->standaloneCheckbox->setCheckState(
            PCM.getItemValueToBool(PCE::Standalone)
                ? Qt::CheckState::Checked
                : Qt::CheckState::Unchecked);
    if (this->onefileCheckbox)
        this->onefileCheckbox->setCheckState(
            PCM.getItemValueToBool(PCE::Onefile)
                ? Qt::CheckState::Checked
                : Qt::CheckState::Unchecked);
    if (this->removeOutputCheckbox)
        this->removeOutputCheckbox->setCheckState(
            PCM.getItemValueToBool(PCE::RemoveOutput)
                ? Qt::CheckState::Checked
                : Qt::CheckState::Unchecked);

    QStringList list = PCM.getItemValue(PCE::DataList).toStringList();
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).isEmpty()) {
            list.removeAt(i);
        }
    }
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(PCE::DataList)), 1,
                              new QTableWidgetItem(list.join(";")));

    this->ltoModeCombobox->setCurrentIndex(
        static_cast<int>(PCM.getItemValue(PCE::LtoMode).value<LTOMode>()));
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
    ui->dataListWidget->clear();
    QStringList dataList = PCM.getItemValueToStringList(PCE::DataList);
    for (const QString &item: dataList) {
        if (item != "") {
            ui->dataListWidget->addItem(item);
        }
    }

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
    ui->defaultPyPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultPythonPath));
    ui->defaultMainPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultMainFilePath));
    ui->defaultOutputPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultOutputPath));
    ui->defaultIconPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultIconPath));
    ui->defaultDataPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultDataPath));

    ui->consoleInputEncodingCombo->setCurrentIndex(
        config.encodingEnumToInt(config.getConfigEncodingEnum(SettingsEnum::ConsoleInputEncoding)));
    ui->consoleOutputEncodingCombo->setCurrentIndex(
        config.encodingEnumToInt(config.getConfigEncodingEnum(SettingsEnum::ConsoleOutputEncoding)));

    ui->packTimerTriggerIntervalSpin->setValue(config.getConfigToInt(SettingsEnum::PackTimerTriggerInterval));
    ui->maxPackLogCountSpin->setValue(config.getConfigToInt(SettingsEnum::MaxPackLogCount));

    ui->showCloseWindowCheckbox->setChecked(config.getConfigToBool(SettingsEnum::IsShowCloseWindow));
    ui->hideOnCloseCheckbox->setChecked(config.getConfigToBool(SettingsEnum::IsHideOnClose));
    if (config.getConfigToBool(SettingsEnum::IsShowCloseWindow)) {
        ui->hideOnCloseCheckbox->setEnabled(false);
    } else {
        ui->hideOnCloseCheckbox->setEnabled(true);
    }

    ui->tempPathEdit->setText(config.getConfigToString(SettingsEnum::TempPath));
}

// Connect functions
void MainWindow::connectStackedWidget() {
    connect(ui->pack_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
        config.writeConfig();
        this->updateUI();
    });
    connect(ui->settings_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
        config.writeConfig();
        this->updateUI();
    });
    connect(ui->export_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2);
        config.writeConfig();
        this->updateUI();
    });
}

void MainWindow::connectMenubar() {
    connect(ui->fileMenu, &QMenu::triggered, this, &MainWindow::onFileMenuTriggered);
    connect(ui->helpMenu, &QMenu::triggered, this, &MainWindow::onHelpMenuTriggered);
    connect(ui->toolMenu, &QMenu::triggered, this, &MainWindow::onToolMenuTriggered);
}

void MainWindow::connectPackPage() {
    // Browse buttons
    // Python file browse button
    connect(ui->pythonFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::PythonPath, QFileDialog::getOpenFileName(
                        this, "Nuitka Studio  Python解释器选择",
                        config.getConfigToString(SettingsEnum::DefaultPythonPath), "exe(*.exe)"));
        ui->pythonFileEdit->setText(PCM.getItemValueToString(PCE::PythonPath));
    });

    // Main file path browse button
    connect(ui->mainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::MainfilePath, QFileDialog::getOpenFileName(
                        this, "Nuitka Studio  主文件选择",
                        config.getConfigToString(SettingsEnum::DefaultMainFilePath),
                        "Python file(*.py)"));
        ui->mainPathEdit->setText(PCM.getItemValueToString(PCE::MainfilePath));
    });

    // Output file path browse button
    connect(ui->outputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::OutputPath, QFileDialog::getExistingDirectory(
                        this, "Nuitka Studio  输出路径",
                        config.getConfigToString(SettingsEnum::DefaultOutputPath),
                        QFileDialog::ShowDirsOnly));
        ui->outputPathEdit->setText(PCM.getItemValueToString(PCE::OutputPath));
    });

    // Project path browse button
    connect(ui->projectPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        PCM.setItem(PCE::ProjectPath, QFileDialog::getExistingDirectory(
                        this, "Nuitka Studio  项目路径",
                        config.getConfigToString(SettingsEnum::DefaultMainFilePath),
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
                EncodingEnum encoding = config.encodingEnumFromInt(index);
                config.setConfigFromEncodingEnum(SettingsEnum::ConsoleInputEncoding, encoding);
            });
    // Console Output Encoding
    connect(ui->consoleOutputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = config.encodingEnumFromInt(index);
                config.setConfigFromEncodingEnum(SettingsEnum::ConsoleOutputEncoding, encoding);
            });
    // Pack Timer Trigger Interval
    connect(ui->packTimerTriggerIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
        config.setConfig(SettingsEnum::PackTimerTriggerInterval, value);
    });
    // Max Pack Log Count
    connect(ui->maxPackLogCountSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
        config.setConfig(SettingsEnum::MaxPackLogCount, value);
    });
    // Is Show Close Window
    connect(ui->showCloseWindowCheckbox, QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            config.setConfigFromBool(SettingsEnum::IsShowCloseWindow, true);
        } else {
            config.setConfigFromBool(SettingsEnum::IsShowCloseWindow, false);
        }
        // Is Enabled hideOnCloseCheckbox
        if (config.getConfigToBool(SettingsEnum::IsShowCloseWindow)) {
            ui->hideOnCloseCheckbox->setEnabled(false);
        } else {
            ui->hideOnCloseCheckbox->setEnabled(true);
        }
    });
    // Is Hide On Close
    connect(ui->hideOnCloseCheckbox, QCheckBox::toggled, this, [=](bool checked) {
        if (checked) {
            config.setConfigFromBool(SettingsEnum::IsHideOnClose, true);
        } else {
            config.setConfigFromBool(SettingsEnum::IsHideOnClose, false);
        }
    });
    // Temp Path
    connect(ui->tempPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setConfig(SettingsEnum::TempPath, text);
    });


    // Default Path Settings
    // Browse Buttons
    // Default Python Path Browse
    connect(ui->defaultPyPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.setConfig(SettingsEnum::DefaultPythonPath,
                         QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认Python解释器路径选择",
                                                           config.getConfigToString(SettingsEnum::DefaultPythonPath),
                                                           QFileDialog::ShowDirsOnly));
        ui->defaultPyPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultPythonPath));
    });
    // Default Main File Path Browse
    connect(ui->defaultMainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.setConfig(SettingsEnum::DefaultMainFilePath,
                         QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认主文件路径选择",
                                                           config.getConfigToString(SettingsEnum::DefaultMainFilePath),
                                                           QFileDialog::ShowDirsOnly));
        ui->defaultMainPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultMainFilePath));
    });
    // Default Output Path Browse
    connect(ui->defaultOutputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.setConfig(SettingsEnum::DefaultOutputPath,
                         QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认输出路径选择",
                                                           config.getConfigToString(SettingsEnum::DefaultOutputPath),
                                                           QFileDialog::ShowDirsOnly));
        ui->defaultOutputPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultOutputPath));
    });
    // Default Icon Path Browse
    connect(ui->defaultIconPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.setConfig(SettingsEnum::DefaultIconPath,
                         QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认图标路径选择",
                                                           config.getConfigToString(SettingsEnum::DefaultIconPath),
                                                           QFileDialog::ShowDirsOnly));
        ui->defaultIconPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultIconPath));
    });
    // Default Data Path Browse
    connect(ui->defaultDataPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        config.setConfig(SettingsEnum::DefaultDataPath,
                         QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认数据路径选择",
                                                           config.getConfigToString(SettingsEnum::DefaultDataPath),
                                                           QFileDialog::ShowDirsOnly));
        ui->defaultDataPathEdit->setText(config.getConfigToString(SettingsEnum::DefaultDataPath));
    });

    // Line Edits
    // Python Edit
    connect(ui->defaultPyPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setConfigFromString(SettingsEnum::DefaultPythonPath, text);
    });
    // Main file Edit
    connect(ui->defaultMainPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setConfigFromString(SettingsEnum::DefaultMainFilePath, text);
    });
    // Output Edit
    connect(ui->defaultOutputPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setConfigFromString(SettingsEnum::DefaultOutputPath, text);
    });
    // Icon Edit
    connect(ui->defaultIconPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setConfigFromString(SettingsEnum::DefaultIconPath, text);
    });
    // Data Edit
    connect(ui->defaultDataPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        config.setConfigFromString(SettingsEnum::DefaultDataPath, text);
    });

    // Save button
    connect(ui->saveSettingsBtn, &QPushButton::clicked, this, [=]() {
        config.writeConfig();
    });
}

void MainWindow::connectExportPage() {
    // Export Button
    connect(ui->exportBtn, &QPushButton::clicked, this, &MainWindow::exportProject);
    // Cell Double-Clicked
    connect(ui->projectTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onProjectTableCellDoubleClicked);
    // Item Changed
    connect(ui->projectTable, &QTableWidget::itemChanged, this, [=](QTableWidgetItem *item) {
        int row = item->row();
        if (row == configListAndUiListMap.value(static_cast<int>(PCE::DataList))) {
            QStringList stringDataList = item->text().split(";");
            PCM.setItem(configListAndUiListInverseMap.value(row), stringDataList);
        } else if (row == configListAndUiListMap.value(static_cast<int>(PCE::LtoMode))) {
            int value = item->text().toInt();
            PCM.setItem(PCE::LtoMode,
                        QVariant::fromValue<LTOMode>(static_cast<LTOMode>(value)));
        } else {
            PCM.setItem(configListAndUiListInverseMap.value(row), item->text());
        }
    });
    // Checkboxes
    connect(this->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        PCM.setItem(PCE::Standalone, state == Qt::CheckState::Checked);
    });
    connect(this->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        PCM.setItem(PCE::Onefile, state == Qt::CheckState::Checked);
    });
    connect(this->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        PCM.setItem(PCE::RemoveOutput, state == Qt::CheckState::Checked);
    });
    // LTO Mode Combobox
    connect(this->ltoModeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        switch (index) {
            case 0:
                PCM.setItem(PCE::LtoMode,
                            QVariant::fromValue<LTOMode>(LTOMode::Auto));
                break;
            case 1:
                PCM.setItem(PCE::LtoMode,
                            QVariant::fromValue<LTOMode>(LTOMode::Yes));
                break;
            case 2:
                PCM.setItem(PCE::LtoMode,
                            QVariant::fromValue<LTOMode>(LTOMode::No));
                break;
            default:
                QMessageBox::warning(this, "Nuitka Studio Warning", "LTO模式值错误");
                break;
        }
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
    // show action
    connect(this->showAction, &QAction::triggered, this, [=]() {
        this->showNormal();
        this->activateWindow();
    });
    // quit action
    connect(this->quitAction, &QAction::triggered, this, [=]() {
        qApp->quit();
    });
}

// Init functions
void MainWindow::initExportPage() {
    // Export
    // 自适应行宽/行高
    ui->projectTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->projectTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    // 表格组件
    this->standaloneCheckbox = new QCheckBox;
    this->onefileCheckbox = new QCheckBox;
    this->removeOutputCheckbox = new QCheckBox;
    this->ltoModeCombobox = new QComboBox;
    this->ltoModeCombobox->addItems(QStringList() << tr("自动") << tr("启用") << tr("禁用"));
    ui->projectTable->setCellWidget(5, 1, this->standaloneCheckbox);
    ui->projectTable->setCellWidget(6, 1, this->onefileCheckbox);
    ui->projectTable->setCellWidget(7, 1, this->removeOutputCheckbox);
    ui->projectTable->setCellWidget(9, 1, this->ltoModeCombobox);
}

void MainWindow::initStatusBar() {
    // Status bar
    this->messageLabel = new QLabel;
    this->messageLabel->setAlignment(Qt::AlignCenter);
    ui->statusbar->addWidget(this->messageLabel);
    ui->statusbar->addPermanentWidget(this->messageLabel, 0);
}

void MainWindow::initTrayMenu() {
    this->trayIcon = new QSystemTrayIcon(QIcon(":/logo"), this);
    this->trayIcon->setToolTip("Nuitka Studio");
    // tray menu
    this->trayMenu = new QMenu(this);
    this->showAction = new QAction("显示", this);
    this->quitAction = new QAction("退出", this);

    trayMenu->addAction(showAction);
    trayMenu->addAction(quitAction);
    trayIcon->setContextMenu(trayMenu);

    trayIcon->show();
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

    this->genPythonPath();
    this->genMainfilePath();
    this->genOutputPath();
    this->genOutputName();
    this->genFileInfo();
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
    if (config.getConfigToBool(SettingsEnum::IsShowCloseWindow)) {
        QDialog dialog(this);
        QVBoxLayout mainLayout;
        dialog.setLayout(&mainLayout);
        // label
        QLabel label("您想要将软件关闭还是最小化至系统托盘");
        label.setAlignment(Qt::AlignCenter);
        mainLayout.addWidget(&label);
        // buttons
        QPushButton trayBtn("最小化至系统托盘");
        mainLayout.addWidget(&trayBtn);
        QPushButton exitBtn("退出软件");
        mainLayout.addWidget(&exitBtn);
        // hide
        QCheckBox hideCheckbox("不再显示该窗口（隐藏后行为可以在设置中看到）");
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
                config.setConfigFromBool(SettingsEnum::IsShowCloseWindow, false);
            } else if (state == Qt::Unchecked) {
                config.setConfigFromBool(SettingsEnum::IsShowCloseWindow, true);
            }
            config.writeConfig();
        });

        dialog.exec();

        if (shouldQuit) {
            config.setConfigFromBool(SettingsEnum::IsHideOnClose, false);
            event->accept();
            config.writeConfig();
            qApp->quit();
        } else if (shouldHide) {
            config.setConfigFromBool(SettingsEnum::IsHideOnClose, true);
            this->hide();
            event->ignore();
            config.writeConfig();
        } else {
            event->ignore();
        }
    } else {
        if (config.getConfigToBool(SettingsEnum::IsHideOnClose)) {
            this->hide();
            event->ignore();
        } else {
            event->accept();
            qApp->quit();
        }
    }
}
