//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : ElaWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->setWindowButtonFlag(ElaAppBarType::NavigationButtonHint, false);
    this->setWindowButtonFlag(ElaAppBarType::RouteBackButtonHint, false);
    this->setWindowButtonFlag(ElaAppBarType::RouteForwardButtonHint, false);

    this->projectConfig = new ProjectConfig;
    this->packTimer = new QTimer(this);
    this->packLog = new QList<PackLog>();
    this->mainTimer = new QTimer(this);
    this->mainTimer->start(1);

    this->packLogModel = new QStringListModel(this);
    this->dataListModel = new QStringListModel(this);

    // Create the temp path
    if (!QDir(config.getConfigToString(SettingsEnum::TempPath)).exists()) {
        if (!QDir().mkpath(config.getConfigToString(SettingsEnum::TempPath))) {
            Logger::warn("缓存文件夹创建失败");
        }
    }

    if (!config.getConfigToString(SettingsEnum::NpfPath).isEmpty()) {
        QString path = config.getConfigToString(SettingsEnum::NpfPath);
        NPFStatusType status = this->projectConfig->loadProject(path);
        if (!this->npfStatusTypeHandler(status, path, false)) {
            if (!GDM.getString(GDIN::NPF_FILE_PATH).isEmpty()) {
                this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
            }
            GDM.setBool(GDIN::IS_OPEN_NPF, true);
        }
    }

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

    if (!GDM.getBool(GDIN::IS_OPEN_NPF)) {
        this->showText("请先新建或打开一个NPF文件再进行操作", -1, Qt::red);
    }

    this->updateUI();

    Logger::info("初始化MainWindow类完成");
}

MainWindow::~MainWindow() {
    config.writeConfig();
    delete this->projectConfig;
    delete this->floatButton;
    delete ui;
}

void MainWindow::startPack() {
    ui->startPackBtn->setEnabled(false);
    this->startPackAction->setEnabled(false);
    ui->stopPackBtn->setEnabled(true);
    this->stopPackAction->setEnabled(true);
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

                QString tempExtractPath = config.getConfigToString(SettingsEnum::TempPath) + "/npf_repack_" + nowString;

                if (!QDir().mkpath(tempExtractPath)) {
                    Logger::error("创建解压文件夹失败，已停止归档");
                    return;
                }

                QStringList extractedFiles =
                        JlCompress::extractDir(GDM.getString(GDIN::NPF_FILE_PATH), tempExtractPath);
                if (extractedFiles.isEmpty()) {
                    ui->consoleOutputEdit->appendPlainText("警告：原 NPF 文件内容解压失败或为空！归档已取消以保护原文件。");
                    return; // 防止覆盖原包
                }

                if (!QFile::exists(tempExtractPath + "/data.json")) {
                    ui->consoleOutputEdit->appendPlainText("致命错误：解压目录中未找到 data.json，停止打包。");
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

                if (Compress::compressDir(tempExtractPath, GDM.getString(GDIN::NPF_FILE_PATH))) {
                    ui->consoleOutputEdit->appendPlainText("日志归档成功！");
                } else {
                    ui->consoleOutputEdit->appendPlainText("归档失败：无法重写 NPF 文件。");
                }

                QDir(tempExtractPath).removeRecursively();

                ui->consoleOutputEdit->appendPlainText(QString("打包日志已存储至<npf_root>/pack_log/" + nowString + ".log"));
                Logger::info(QString("打包日志已存储至<npf_root>/pack_log/" + nowString + ".log"));
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
    this->packTimer->start(config.getConfigToInt(SettingsEnum::PackTimerTriggerInterval));
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
                                                config.getConfigToString(SettingsEnum::DefaultDataPath),
                                                "Nuitka Project File(*.npf);;All files(*)");
    if (path.isEmpty()) {
        return;
    }
    NPFStatusType status = this->projectConfig->loadProject(path);
    if (this->npfStatusTypeHandler(status, path)) {
        return;
    }
    // Update UI
    this->updateUI();
    if (!path.isEmpty()) {
        this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
    }
    GDM.setBool(GDIN::IS_OPEN_NPF, true);
    this->clearText();
}

void MainWindow::exportProject() {
    QString path = QFileDialog::getSaveFileName(this, "Nuitka Studio  导出项目文件",
                                                config.getConfigToString(SettingsEnum::DefaultDataPath),
                                                "Nuitka Project File(*.npf);;All files(*)");
    if (path.isEmpty()) {
        return;
    }
    NPFStatusType status = this->projectConfig->saveProject(path);
    if (this->npfStatusTypeHandler(status, path)) {
        return;
    }
    this->updateUI();
    if (!path.isEmpty()) {
        this->setWindowTitle(path.split("/").last() + " - Nuitka Studio");
    }
    GDM.setBool(GDIN::IS_OPEN_NPF, true);
}

// Slots
void MainWindow::onAddDataFileItemClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件",
                                                    config.getConfigToString(SettingsEnum::DefaultDataPath));
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
                                                        config.getConfigToString(SettingsEnum::DefaultDataPath),
                                                        QFileDialog::ShowDirsOnly);
    if (dirPath.isEmpty()) {
        return;
    }

    QStringList dataList = this->dataListModel->stringList();
    dataList << dirPath;
    this->dataListModel->setStringList(dataList);
    PCM.appendItemToStringList(PCE::DataList, dirPath);
}

void MainWindow::onRemoveItemClicked() {
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

    if (text == "新建(&N)") {
        QString path = QFileDialog::getSaveFileName(this, "Nuitka Studio 新建NPF文件",
                                                    config.getConfigToString(SettingsEnum::DefaultDataPath),
                                                    "Nuitka Project File(*.npf);;All files(*)");
        if (path.isEmpty()) {
            return;
        }
        PCM.setDefaultValue();
        if (this->npfStatusTypeHandler(this->projectConfig->saveProject(path), path)) {
            return;
        }
        GDM.setString(GDIN::NPF_FILE_PATH, path);
        GDM.setBool(GDIN::IS_OPEN_NPF, true);
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
    } else if (text == "打开(&O)") {
        this->importProject();
        this->clearText();
    } else if (text == "保存(&S)") {
        this->npfStatusTypeHandler(this->projectConfig->saveProject(GDM.getString(GDIN::NPF_FILE_PATH)),
                                   GDM.getString(GDIN::NPF_FILE_PATH));
    } else if (text == "另存为(&A)") {
        this->exportProject();
    } else if (text == "关闭文件(&C)") {
        int choose = QMessageBox::question(this, "Nuitka Studio", "关闭后未保存的数据将会丢失，是否确认关闭");
        if (choose == QMessageBox::Yes) {
            PCM.setDefaultValue();
            GDM.setString(GDIN::NPF_FILE_PATH, "");
            GDM.setBool(GDIN::IS_OPEN_NPF, false);
            config.setConfigFromString(SettingsEnum::NpfPath, "");
            this->setWindowTitle("Nuitka Studio");
            this->updateUI();
        }
    }
}

void MainWindow::onHelpMenuTriggered(QAction *action) {
    QString text = action->text();
    Logger::info(QString("菜单：帮助, 菜单项 %1 触发triggered事件").arg(text));

    if (text == "帮助(&H)") {
        QDesktopServices::openUrl(QUrl("https://github.com/Redrch/NuitkaStudio"));
    } else if (text == "关于(&A)") {
        auto *aboutWindow = new AboutWindow(this);
        aboutWindow->exec();
    }
}

// Update UI functions
void MainWindow::updateUI() {
    this->updatePackUI();
    this->updateSettingsUI();
    if (!GDM.getString(GDIN::NPF_FILE_PATH).isEmpty()) {
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

void MainWindow::updatePackLogUI() {
    this->readPackLog();
    QStringList logStringList;
    for (PackLog &log: *this->packLog) {
        logStringList.append(log.logFileName);
    }
    this->packLogModel->setStringList(logStringList);
    if (this->packLog->count() > 0) {
        ui->packLogContent->setPlainText(this->packLog->at(0).logContent);
    }
}

// Connect functions
void MainWindow::connectStackedWidget() {
    connect(this->menuBar, &ElaMenuBar::triggered, this, [=](QAction *action) {
        QString text = action->text();
        if (text == "打包") {
            ui->stackedWidget->setCurrentIndex(0);
        } else if (text == "设置") {
            ui->stackedWidget->setCurrentIndex(1);
        } else if (text == "打包日志") {
            ui->stackedWidget->setCurrentIndex(2);
        } else if (text == "") {
            this->hide();
            this->floatButton->show();
        }
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
    connect(ui->packTimerTriggerIntervalSpin, QOverload<int>::of(&ElaSpinBox::valueChanged), this, [=](int value) {
        config.setConfig(SettingsEnum::PackTimerTriggerInterval, value);
    });
    // Max Pack Log Count
    connect(ui->maxPackLogCountSpin, QOverload<int>::of(&ElaSpinBox::valueChanged), this, [=](int value) {
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
        if (valueName == GDIN::IS_OPEN_NPF) {
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
    });

    connect(this->floatButton, &FloatButton::startPack, this, &MainWindow::startPack);
    connect(this->floatButton, &FloatButton::stopPack, this, &MainWindow::stopPack);
    connect(this->floatButton, &FloatButton::showMainWindow, this, [=]() {
        this->floatButton->hide();
        this->showNormal();
        this->activateWindow();
    });
}

void MainWindow::connectPackLog() {
    connect(ui->packLogFileList, &ElaListView::clicked, this, [=](const QModelIndex &index) {
        int row = index.row();
        const QString &content = this->packLog->at(row).logContent;
        ui->packLogContent->setPlainText(content);
    });
}

// Init functions
void MainWindow::initUI() {
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
    if (!GDM.getBool(GDIN::IS_OPEN_NPF)) {
        this->noEnableInput();
    }

    // controls
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
    // customLayout->addStretch();
    customLayout->addWidget(this->menuBar);
    customLayout->addStretch();
    this->setCustomWidget(ElaAppBarType::MiddleArea, customWidget);

    this->menuBar->addElaIconAction(ElaIconType::BoxesPacking, "打包");
    this->menuBar->addElaIconAction(ElaIconType::Gear, "设置");
    this->menuBar->addElaIconAction(ElaIconType::File, "打包日志");
    this->menuBar->addElaIconAction(ElaIconType::SquareXmark, "");
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

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);

    int x = (this->width() - this->topTextLabel->sizeHint().width()) / 2;
    int y = 30;
    this->topTextLabel->move(x, y);
    this->topTextLabel->raise();
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
    if (GDM.getString(GDIN::NPF_FILE_PATH).isEmpty()) {
        Logger::warn("NPF文件路径为空，无法调用MainWindow::readPackLog函数");
        return;
    }
    QString nowString = QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss");
    QString zipTempPath = config.getConfigToString(SettingsEnum::TempPath) + "/npf_repack_" + nowString;
    QString packLogPath = zipTempPath + "/pack_log";

    Compress::extractZip(GDM.getString(GDIN::NPF_FILE_PATH), zipTempPath);
    QStringList logList = QDir(packLogPath).entryList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString &logFileName: logList) {
        QFile file(packLogPath + "/" + logFileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::warning(this, "Nuitka Studio Warning", "无法打开打包日志文件" + logFileName);
            Logger::warn("无法打开打包日志文件" + logFileName);
            continue;
        }
        QString log = QString::fromUtf8(file.readAll());
        this->packLog->append(PackLog(logFileName, log));
    }
    // remove temp files
    QDir(zipTempPath).removeRecursively();
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
        case NPFStatusType::NPFRight:
            return false;

    }
    return true;
}
