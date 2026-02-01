//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"

#include <complex>

#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->projectConfig = new ProjectConfig(this);

    if (!QFile::exists(Config::instance().getConfigPath())) {
        Config::instance().writeConfig();
    }
    Config::instance().readConfig();

    this->packTimer = new QTimer(this);

    // Init UI
    this->initExportPage();
    this->initStatusBar();

    // Connect signal and slot
    this->connectStackedWidget();
    this->connectMenubar();
    this->connectPackPage();
    this->connectSettingsPage();
    this->connectExportPage();

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
    timer.start();

    this->packProcess = new QProcess(this);

    // Signals and slots
    // 合并普通和错误输出
    this->packProcess->setProcessChannelMode(QProcess::MergedChannels);
    // output
    connect(this->packProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString out = QString::fromLocal8Bit(this->packProcess->readAllStandardOutput());
        ui->consoleOutputEdit->append(out);
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
            });
    // error occurred
    connect(this->packProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        qWarning() << "command error: " << error;
        ui->consoleOutputEdit->append("Error: " + Utils::processErrorToString(error));
        Logger::error("Error: " + Utils::processErrorToString(error));
    });

    if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath).isEmpty()) {
        ui->consoleOutputEdit->append("python解释器路径为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::MainfilePath).isEmpty()) {
        ui->consoleOutputEdit->append("主文件路径为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputPath).isEmpty()) {
        ui->consoleOutputEdit->append("输出目录为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputFilename).isEmpty()) {
        ui->consoleOutputEdit->append("输出文件名为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->startPackBtn->setEnabled(false);
        return;
    }

    // build args
    QStringList args = QStringList();
    args << "-m" << "nuitka";
    if (ProjectConfigManager::instance().getItemValueToBool(ConfigValue::Standalone)) {
        args << "--standalone";
    }
    if (ProjectConfigManager::instance().getItemValueToBool(ConfigValue::Onefile)) {
        args << "--onefile";
    }
    if (ProjectConfigManager::instance().getItemValueToBool(ConfigValue::RemoveOutput)) {
        args << "--remove-output";
    }

    // LTO
    switch (ProjectConfigManager::instance().getItem(ConfigValue::LtoMode)->get_itemValue().value<LTOMode>()) {
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

    args << ProjectConfigManager::instance().getItemValueToString(ConfigValue::MainfilePath);
    args << "--output-dir=" + ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputPath);
    args << "--output-filename=" + ProjectConfigManager::instance().getItemValueToString(
        ConfigValue::OutputFilename);

    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::IconPath).isEmpty()) {
        args << "--windows-icon-from-ico=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::IconPath);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::FileVersion).isEmpty()) {
        args << "--file-version=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::FileVersion);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::Company).isEmpty()) {
        args << "--company-name=" + ProjectConfigManager::instance().getItemValueToString(ConfigValue::Company);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProductName).isEmpty()) {
        args << "--product-name=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::ProductName);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProductVersion).isEmpty()) {
        args << "--product-version=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::ProductVersion);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::FileDescription).isEmpty()) {
        args << "--file-description=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::FileDescription);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::LegalCopyright).isEmpty()) {
        args << "--copyright=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::LegalCopyright);
    }
    if (!ProjectConfigManager::instance().getItemValueToString(ConfigValue::LegalTrademarks).isEmpty()) {
        args << "--trademarks=" + ProjectConfigManager::instance().getItemValueToString(
            ConfigValue::LegalTrademarks);
    }

    this->packProcess->start(
        ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath), args);

    // console output
    QString outputString = QString("-------------- 开始打包 %1 -------------").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    ui->consoleOutputEdit->append(outputString);
    // pack timer
    this->startPackTime = QDateTime::currentDateTime();
    this->packTimer->start(Config::instance().getPackTimerTriggerInterval());
    // console output
    ui->consoleOutputEdit->append(
        ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath) + " " + args.
        join(" "));
    Logger::info(
        "开始打包  打包命令: " + QString(
            ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath) + " " + args.
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
    this->projectConfig->importProject();
    // Update UI
    this->updateUI();
}

void MainWindow::exportProject() {
    this->projectConfig->exportProject();
}

void MainWindow::newProject() {
    auto *newProjectWindow = new NewProjectWindow(this);
    auto *process = new QProcess(this);
    newProjectWindow->setWindowFlags(newProjectWindow->windowFlags() | Qt::Window);
    newProjectWindow->exec();
    this->messageLabel->setText(
        QString("正在为项目%1安装nuitka...").arg(
            ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName)));
    newProjectWindow->installNuitka(process);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                this->messageLabel->setText(QString("项目%1的nuitka安装完毕").arg(
                    ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName)));
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
                                                    Config::instance().getDefaultDataPath());
    if (filePath == "") {
        return;
    }
    ui->dataListWidget->addItem(filePath);
    ProjectConfigManager::instance().appendItemToStringList(ConfigValue::DataList, filePath);
}

void MainWindow::onAddDataDirItemClicked() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                        Config::instance().getDefaultDataPath(),
                                                        QFileDialog::ShowDirsOnly);
    if (dirPath == "") {
        return;
    }

    ui->dataListWidget->addItem(dirPath);
    ProjectConfigManager::instance().appendItemToStringList(ConfigValue::DataList, dirPath);
}

void MainWindow::onRemoveItemClicked() {
    QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
    if (removeItem == nullptr) {
        return;
    }

    ProjectConfigManager::instance().removeItemFromStringList(ConfigValue::DataList, removeItem->text());
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
void MainWindow::updateUI() {
    this->updateExportTable();
    this->updatePackUI();

    ui->defaultPyPathEdit->setText(Config::instance().getDefaultPythonPath());
    ui->defaultMainPathEdit->setText(Config::instance().getDefaultMainFilePath());
    ui->defaultOutputPathEdit->setText(Config::instance().getDefaultOutputPath());
    ui->defaultIconPathEdit->setText(Config::instance().getDefaultIconPath());
    ui->defaultDataPathEdit->setText(Config::instance().getDefaultDataPath());

    ui->consoleInputEncodingCombo->setCurrentIndex(
        Config::instance().encodingEnumToInt(Config::instance().getConsoleInputEncoding()));
    ui->consoleOutputEncodingCombo->setCurrentIndex(
        Config::instance().encodingEnumToInt(Config::instance().getConsoleOutputEncoding()));
    Logger::info("刷新UI");
}

void MainWindow::updateExportTable() {
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::PythonPath)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::MainfilePath)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::MainfilePath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::OutputPath)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::OutputFilename)), 1, new QTableWidgetItem(
            ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputFilename)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::IconPath)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::IconPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::ProjectPath)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath)));
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::ProjectName)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName)));

    // file info
    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::FileVersion)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::FileVersion)));

    ui->projectTable->setItem(
        configListAndUiListMap.value(static_cast<int>(ConfigValue::Company)), 1,
        new QTableWidgetItem(ProjectConfigManager::instance().getItemValueToString(ConfigValue::Company)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(ConfigValue::ProductName)), 1,
                              new QTableWidgetItem(
                                  ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProductName)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(ConfigValue::ProductVersion)), 1,
                              new QTableWidgetItem(
                                  ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProductVersion)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(ConfigValue::FileDescription)), 1,
                              new QTableWidgetItem(
                                  ProjectConfigManager::instance().getItemValueToString(ConfigValue::FileDescription)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(ConfigValue::LegalCopyright)), 1,
                              new QTableWidgetItem(
                                  ProjectConfigManager::instance().getItemValueToString(ConfigValue::LegalCopyright)));
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(ConfigValue::LegalTrademarks)), 1,
                              new QTableWidgetItem(
                                  ProjectConfigManager::instance().getItemValueToString(ConfigValue::LegalTrademarks)));


    if (this->standaloneCheckbox)
        this->standaloneCheckbox->setCheckState(
            ProjectConfigManager::instance().getItemValueToBool(ConfigValue::Standalone)
                ? Qt::CheckState::Checked
                : Qt::CheckState::Unchecked);
    if (this->onefileCheckbox)
        this->onefileCheckbox->setCheckState(
            ProjectConfigManager::instance().getItemValueToBool(ConfigValue::Onefile)
                ? Qt::CheckState::Checked
                : Qt::CheckState::Unchecked);
    if (this->removeOutputCheckbox)
        this->removeOutputCheckbox->setCheckState(
            ProjectConfigManager::instance().getItemValueToBool(ConfigValue::RemoveOutput)
                ? Qt::CheckState::Checked
                : Qt::CheckState::Unchecked);

    QStringList list = ProjectConfigManager::instance().getItemValue(ConfigValue::DataList).toStringList();
    for (int i = 0; i < list.size(); i++) {
        if (list.at(i).isEmpty()) {
            list.removeAt(i);
        }
    }
    ui->projectTable->setItem(configListAndUiListMap.value(static_cast<int>(ConfigValue::DataList)), 1,
                              new QTableWidgetItem(list.join(";")));

    this->ltoModeCombobox->setCurrentIndex(
        static_cast<int>(ProjectConfigManager::instance().getItemValue(ConfigValue::LtoMode).value<LTOMode>()));
}

void MainWindow::updatePackUI() {
    ui->pythonFileEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath));
    ui->mainPathEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::MainfilePath));
    ui->outputPathEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputPath));
    ui->outputFileEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputFilename));
    ui->iconFileEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::IconPath));
    ui->projectPathEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath));
    ui->projectNameEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName));

    ui->standaloneCheckbox->setCheckState(
        ProjectConfigManager::instance().getItemValueToBool(ConfigValue::Standalone)
            ? Qt::CheckState::Checked
            : Qt::CheckState::Unchecked);
    ui->onefileCheckbox->setCheckState(ProjectConfigManager::instance().getItemValueToBool(ConfigValue::Onefile)
                                           ? Qt::CheckState::Checked
                                           : Qt::CheckState::Unchecked);
    ui->removeOutputCheckbox->setCheckState(
        ProjectConfigManager::instance().getItemValueToBool(ConfigValue::RemoveOutput)
            ? Qt::CheckState::Checked
            : Qt::CheckState::Unchecked);
    // LTO
    switch (ProjectConfigManager::instance().getItemValue(ConfigValue::LtoMode).value<LTOMode>()) {
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
    QStringList dataList = ProjectConfigManager::instance().getItemValueToStringList(ConfigValue::DataList);
    for (const QString &item: dataList) {
        if (item != "") {
            ui->dataListWidget->addItem(item);
        }
    }

    // File info
    ui->fileVersionEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::FileVersion));
    ui->companyEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::Company));
    ui->productNameEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProductName));
    ui->productVersionEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProductVersion));

    ui->fileDescriptitonEdit->setText(
        ProjectConfigManager::instance().getItemValueToString(ConfigValue::FileDescription));
    ui->legalCopyrightEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::LegalCopyright));
    ui->legalTrademarksEdit->setText(
        ProjectConfigManager::instance().getItemValueToString(ConfigValue::LegalTrademarks));
}

// Connect functions
void MainWindow::connectStackedWidget() {
    connect(ui->pack_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
        Config::instance().writeConfig();
        this->updateUI();
    });
    connect(ui->settings_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
        Config::instance().writeConfig();
        this->updateUI();
    });
    connect(ui->export_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2);
        Config::instance().writeConfig();
        this->updateUI();
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
        ProjectConfigManager::instance().setItem(ConfigValue::PythonPath, QFileDialog::getOpenFileName(
                                                     this, "Nuitka Studio  Python解释器选择",
                                                     Config::instance().getDefaultPythonPath(), "exe(*.exe)"));
        ui->pythonFileEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::PythonPath));
    });

    // Main file path browse button
    connect(ui->mainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        ProjectConfigManager::instance().setItem(ConfigValue::MainfilePath, QFileDialog::getOpenFileName(
                                                     this, "Nuitka Studio  主文件选择",
                                                     Config::instance().getDefaultMainFilePath(),
                                                     "Python file(*.py)"));
        ui->mainPathEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::MainfilePath));
    });

    // Output file path browse button
    connect(ui->outputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        ProjectConfigManager::instance().setItem(ConfigValue::OutputPath, QFileDialog::getExistingDirectory(
                                                     this, "Nuitka Studio  输出路径",
                                                     Config::instance().getDefaultOutputPath(),
                                                     QFileDialog::ShowDirsOnly));
        ui->outputPathEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::OutputPath));
    });

    // Project path browse button
    connect(ui->projectPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        ProjectConfigManager::instance().setItem(ConfigValue::ProjectPath, QFileDialog::getExistingDirectory(
                                                     this, "Nuitka Studio  项目路径",
                                                     Config::instance().getDefaultMainFilePath(),
                                                     QFileDialog::ShowDirsOnly));
        ui->projectPathEdit->setText(
            ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath));

        ProjectConfigManager::instance().setItem(ConfigValue::ProjectName,
                                                 ProjectConfigManager::instance().getItemValueToString(
                                                     ConfigValue::ProjectPath).split("/").last());
        ui->projectNameEdit->setText(
            ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName));
        if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath).isEmpty()) return;

        this->genData();
    });

    // Edits
    // Python file edit
    connect(ui->pythonFileEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::PythonPath, text);
    });

    // Main file path edit
    connect(ui->mainPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::MainfilePath, text);
    });

    // Output file path edit
    connect(ui->outputPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::OutputPath, text);
    });

    // Output file name edit
    connect(ui->outputFileEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::OutputFilename, text);
    });

    // Project path edit
    connect(ui->projectPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::ProjectPath, text);
    });

    // Project name edit
    connect(ui->projectNameEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::ProjectName, text);
    });

    // Build Settings
    // Standalone
    connect(ui->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            ProjectConfigManager::instance().setItem(ConfigValue::Standalone, false);
        } else if (state == Qt::Checked) {
            ProjectConfigManager::instance().setItem(ConfigValue::Standalone, true);
        }
    });
    // Onefile
    connect(ui->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            ProjectConfigManager::instance().setItem(ConfigValue::Onefile, false);
        } else if (state == Qt::Checked) {
            ProjectConfigManager::instance().setItem(ConfigValue::Onefile, true);
        }
    });
    // Remove Output
    connect(ui->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            ProjectConfigManager::instance().setItem(ConfigValue::RemoveOutput, false);
        } else if (state == Qt::Checked) {
            ProjectConfigManager::instance().setItem(ConfigValue::RemoveOutput, true);
        }
    });

    // LTO Mode Checkbox
    // No
    connect(ui->ltoNo, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
                                                     QVariant::fromValue<LTOMode>(LTOMode::No));
        }
    });
    // Yes
    connect(ui->ltoYes, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoYes->checkState() == Qt::CheckState::Checked) {
            ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
                                                     QVariant::fromValue<LTOMode>(LTOMode::Yes));
        }
    });
    // Auto
    connect(ui->ltoAuto, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoAuto->checkState() == Qt::CheckState::Checked) {
            ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
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
        ProjectConfigManager::instance().setItem(ConfigValue::IconPath, QFileDialog::getOpenFileName(
                                                     this, "Nuitka Studio  图标路径", "C:\\",
                                                     "Icon file(*.jpg *.jpeg *.png *.ico);;All files(*)"));
        ui->iconFileEdit->setText(ProjectConfigManager::instance().getItemValueToString(ConfigValue::IconPath));
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
        ProjectConfigManager::instance().setItem(ConfigValue::FileVersion, text);
        this->genFileInfo();
        this->updateUI();
    });
    connect(ui->companyEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::Company, text);
    });
    connect(ui->productNameEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::ProductName, text);
    });
    connect(ui->productVersionEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::ProductVersion, text);
    });
    connect(ui->fileDescriptitonEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::FileDescription, text);
    });
    connect(ui->legalCopyrightEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::LegalCopyright, text);
    });
    connect(ui->legalTrademarksEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        ProjectConfigManager::instance().setItem(ConfigValue::LegalTrademarks, text);
    });
}

void MainWindow::connectSettingsPage() {
    // General Settings
    // Console Input Encoding
    connect(ui->consoleInputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = Config::instance().encodingEnumFromInt(index);
                Config::instance().setConsoleInputEncoding(encoding);
            });
    // Console Output Encoding
    connect(ui->consoleOutputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = Config::instance().encodingEnumFromInt(index);
                Config::instance().setConsoleOutputEncoding(encoding);
            });
    // Pack Timer Trigger Interval
    connect(ui->packTimerTriggerIntervalSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
        Config::instance().setPackTimerTriggerInterval(value);
    });


    // Default Path Settings
    // Browse Buttons
    // Default Python Path Browse
    connect(ui->defaultPyPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        Config::instance().setDefaultPythonPath(
            QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认Python解释器路径选择",
                                              Config::instance().getDefaultPythonPath(),
                                              QFileDialog::ShowDirsOnly));
        ui->defaultPyPathEdit->setText(Config::instance().getDefaultPythonPath());
    });
    // Default Main File Path Browse
    connect(ui->defaultMainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        Config::instance().setDefaultMainFilePath(
            QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认主文件路径选择",
                                              Config::instance().getDefaultMainFilePath(),
                                              QFileDialog::ShowDirsOnly));
        ui->defaultMainPathEdit->setText(Config::instance().getDefaultMainFilePath());
    });
    // Default Output Path Browse
    connect(ui->defaultOutputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        Config::instance().setDefaultOutputPath(
            QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认输出路径选择",
                                              Config::instance().getDefaultIconPath(),
                                              QFileDialog::ShowDirsOnly));
        ui->defaultOutputPathEdit->setText(Config::instance().getDefaultOutputPath());
    });
    // Default Icon Path Browse
    connect(ui->defaultIconPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        Config::instance().setDefaultIconPath(
            QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认图标路径选择",
                                              Config::instance().getDefaultIconPath(),
                                              QFileDialog::ShowDirsOnly));
        ui->defaultIconPathEdit->setText(Config::instance().getDefaultIconPath());
    });

    // Line Edits
    // Python Edit
    connect(ui->defaultPyPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        Config::instance().setDefaultPythonPath(text);
    });
    // Main file Edit
    connect(ui->defaultMainPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        Config::instance().setDefaultMainFilePath(text);
    });
    // Output Edit
    connect(ui->defaultOutputPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        Config::instance().setDefaultOutputPath(text);
    });
    // Icon Edit
    connect(ui->defaultIconPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        Config::instance().setDefaultIconPath(text);
    });
    // Data Edit
    connect(ui->defaultDataPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        Config::instance().setDefaultDataPath(text);
    });

    // Save button
    connect(ui->saveSettingsBtn, &QPushButton::clicked, this, [=]() {
        Config::instance().writeConfig();
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
        if (row == configListAndUiListMap.value(static_cast<int>(ConfigValue::DataList))) {
            QStringList stringDataList = item->text().split(";");
            ProjectConfigManager::instance().setItem(configListAndUiListInverseMap.value(row), stringDataList);
        } else if (row == configListAndUiListMap.value(static_cast<int>(ConfigValue::LtoMode))) {
            int value = item->text().toInt();
            ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
                                                     QVariant::fromValue<LTOMode>(static_cast<LTOMode>(value)));
        } else {
            ProjectConfigManager::instance().setItem(configListAndUiListInverseMap.value(row), item->text());
        }
    });
    // Checkboxes
    connect(this->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        ProjectConfigManager::instance().setItem(ConfigValue::Standalone, state == Qt::CheckState::Checked);
    });
    connect(this->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        ProjectConfigManager::instance().setItem(ConfigValue::Onefile, state == Qt::CheckState::Checked);
    });
    connect(this->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        ProjectConfigManager::instance().setItem(ConfigValue::RemoveOutput, state == Qt::CheckState::Checked);
    });
    // LTO Mode Combobox
    connect(this->ltoModeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        switch (index) {
            case 0:
                ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
                                                         QVariant::fromValue<LTOMode>(LTOMode::Auto));
                break;
            case 1:
                ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
                                                         QVariant::fromValue<LTOMode>(LTOMode::Yes));
                break;
            case 2:
                ProjectConfigManager::instance().setItem(ConfigValue::LtoMode,
                                                         QVariant::fromValue<LTOMode>(LTOMode::No));
                break;
            default:
                QMessageBox::warning(this, "Nuitka Studio Warning", "LTO模式值错误");
                break;
        }
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

// gen path functions
void MainWindow::genData(bool isUpdateUI) {
    if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath).isEmpty()) {
        QMessageBox::warning(this, "Nuitka Studio Warning", "请填写项目路径");
        return;
    }
    if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName).isEmpty()) {
        ProjectConfigManager::instance().setItem(ConfigValue::ProjectName,
                                                 ProjectConfigManager::instance().getItemValueToString(
                                                     ConfigValue::ProjectPath).split("/").last());
        if (ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectName).isEmpty()) {
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
    QDir projectDir(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath));
    if (projectDir.exists(
        ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath) + "/.venv")) {
        ProjectConfigManager::instance().setItem(ConfigValue::PythonPath,
                                                 ProjectConfigManager::instance().getItemValueToString(
                                                     ConfigValue::ProjectPath) + "/.venv" + "/Scripts" +
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
                    ProjectConfigManager::instance().setItem(ConfigValue::PythonPath, fi.filePath());
                }
            }
        }
    }
}

void MainWindow::genMainfilePath() {
    QDir projectDir(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath));
    if (projectDir.exists(ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath) + "/src")
        || projectDir.exists(
            ProjectConfigManager::instance().getItemValueToString(ConfigValue::ProjectPath) + "/source")) {
        ProjectConfigManager::instance().setItem(ConfigValue::MainfilePath,
                                                 ProjectConfigManager::instance().getItemValueToString(
                                                     ConfigValue::ProjectPath) + "/src/main.py");
    } else {
        ProjectConfigManager::instance().setItem(ConfigValue::MainfilePath,
                                                 ProjectConfigManager::instance().getItemValueToString(
                                                     ConfigValue::ProjectPath) +
                                                 "/main.py");
    }
}

void MainWindow::genOutputPath() {
    ProjectConfigManager::instance().setItem(ConfigValue::OutputPath,
                                             ProjectConfigManager::instance().getItemValueToString(
                                                 ConfigValue::ProjectPath) +
                                             "/output");
}

void MainWindow::genOutputName() {
    ProjectConfigManager::instance().setItem(ConfigValue::OutputFilename,
                                             ProjectConfigManager::instance().getItemValueToString(
                                                 ConfigValue::ProjectName) +
                                             ".exe");
}

void MainWindow::genFileInfo() {
    ProjectConfigManager::instance().setItem(ConfigValue::ProductName,
                                             ProjectConfigManager::instance().getItemValueToString(
                                                 ConfigValue::ProjectName));
}
