//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->data = new ProjectConfigData;
    this->projectConfig = new ProjectConfig(this->data, this);

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

    spdlog::info("初始化MainWindow类完成");
}

MainWindow::~MainWindow() {
    delete this->data;
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
        spdlog::info(out.toStdString());
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
                spdlog::info(QString("----------- 打包结束 耗时: %1 ----------").arg(timeString).toStdString());
                this->packProcess->deleteLater();
                this->packTimer->stop();
                ui->startPackBtn->setEnabled(true);
                ui->stopPackBtn->setEnabled(false);
            });
    // error occurred
    connect(this->packProcess, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        qWarning() << "command error: " << error;
        ui->consoleOutputEdit->append("Error: " + Utils::processErrorToString(error));
        spdlog::error("Error: " + Utils::processErrorToString(error).toStdString());
    });

    if (this->data->pythonPath == "") {
        ui->consoleOutputEdit->append("python解释器路径为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (this->data->mainFilePath == "") {
        ui->consoleOutputEdit->append("主文件路径为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (this->data->outputPath == "") {
        ui->consoleOutputEdit->append("输出目录为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->stopPackBtn->setEnabled(false);
        return;
    }
    if (this->data->outputFilename == "") {
        ui->consoleOutputEdit->append("输出文件名为必填项");
        ui->startPackBtn->setEnabled(true);
        ui->startPackBtn->setEnabled(false);
        return;
    }

    // build args
    QStringList args = QStringList();
    args << "-m" << "nuitka";
    if (this->data->standalone) {
        args << "--standalone";
    }
    if (this->data->onefile) {
        args << "--onefile";
    }
    if (this->data->removeOutput) {
        args << "--remove-output";
    }

    // LTO
    switch (this->data->ltoMode) {
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

    args << this->data->mainFilePath;
    args << "--output-dir=" + this->data->outputPath;
    args << "--output-filename=" + this->data->outputFilename;

    if (this->data->iconPath != QString("")) {
        args << "--windows-icon-from-ico=" + this->data->iconPath;
    }

    this->packProcess->start(this->data->pythonPath, args);

    // console output
    QString outputString = QString("-------------- 开始打包 %1 -------------").arg(
        QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    ui->consoleOutputEdit->append(outputString);
    // pack timer
    this->startPackTime = QDateTime::currentDateTime();
    this->packTimer->start(Config::instance().getPackTimerTriggerInterval());
    // use time obj
    ui->consoleOutputEdit->append(this->data->pythonPath + " " + args.join(" "));
    spdlog::info("开始打包  打包命令: " + QString(this->data->pythonPath + " " + args.join(" ")).toStdString());
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
    auto* newProjectWindow = new NewProjectWindow(this);
    newProjectWindow->setWindowFlags(newProjectWindow->windowFlags() | Qt::Window);
    newProjectWindow->setAttribute(Qt::WA_DeleteOnClose);
    newProjectWindow->show();
}

// Slots
void MainWindow::onAddDataFileItemClicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件",
                                                    Config::instance().getDefaultDataPath());
    if (filePath == "") {
        return;
    }
    ui->dataListWidget->addItem(filePath);
    this->data->dataList.append(filePath);

    Logger::debug(this->data->dataList[0]);
    Logger::debug(this->data->dataList[1]);
}

void MainWindow::onAddDataDirItemClicked() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                        Config::instance().getDefaultDataPath(),
                                                        QFileDialog::ShowDirsOnly);
    if (dirPath == "") {
        return;
    }

    ui->dataListWidget->addItem(dirPath);
    this->data->dataList.append(dirPath);
}

void MainWindow::onRemoveItemClicked() {
    QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
    if (removeItem == nullptr) {
        return;
    }

    this->data->dataList.removeOne(removeItem->text());
    delete removeItem;
}

void MainWindow::onProjectTableCellDoubleClicked(int row, int column) {
    // Data List
    if (row == 8 and column == 1) {
        auto *dataListWindow = new ExportDataListWindow(this);
        dataListWindow->setWindowFlags(dataListWindow->windowFlags() | Qt::Window);
        dataListWindow->setAttribute(Qt::WA_DeleteOnClose);
        dataListWindow->setDataList(this->data->dataList);

        connect(dataListWindow, &ExportDataListWindow::dataListChanged, this, [=](const QList<QString> &newDataList) {
            this->data->dataList = newDataList;
            this->updateUI();
        });

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
    spdlog::info("刷新UI");
}

void MainWindow::updateExportTable() {
    ui->projectTable->setItem(0, 1, new QTableWidgetItem(this->data->pythonPath));
    ui->projectTable->setItem(1, 1, new QTableWidgetItem(this->data->mainFilePath));
    ui->projectTable->setItem(2, 1, new QTableWidgetItem(this->data->outputPath));
    ui->projectTable->setItem(3, 1, new QTableWidgetItem(this->data->outputFilename));
    ui->projectTable->setItem(4, 1, new QTableWidgetItem(this->data->iconPath));
    if (this->standaloneCheckbox)
        this->standaloneCheckbox->setCheckState(
            this->data->standalone ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    if (this->onefileCheckbox)
        this->onefileCheckbox->setCheckState(
            this->data->onefile ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    if (this->removeOutputCheckbox)
        this->removeOutputCheckbox->setCheckState(
            this->data->removeOutput ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->projectTable->setItem(8, 1, new QTableWidgetItem(this->data->dataList.join(";")));
    int index = 0;
    switch (this->data->ltoMode) {
        case LTOMode::Auto:
            index = 0;
            break;
        case LTOMode::Yes:
            index = 1;
            break;
        case LTOMode::No:
            index = 2;
            break;
    }
    this->ltoModeCombobox->setCurrentIndex(index);
}

void MainWindow::updatePackUI() {
    ui->pythonFileEdit->setText(this->data->pythonPath);
    ui->mainPathEdit->setText(this->data->mainFilePath);
    ui->outputPathEdit->setText(this->data->outputPath);
    ui->outputFileEdit->setText(this->data->outputFilename);
    ui->iconFileEdit->setText(this->data->iconPath);
    ui->standaloneCheckbox->setCheckState(this->data->standalone ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->onefileCheckbox->setCheckState(this->data->onefile ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->removeOutputCheckbox->setCheckState(this->data->removeOutput ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    // LTO
    switch (this->data->ltoMode) {
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
    for (const QString &item: this->data->dataList) {
        if (item != "") {
            ui->dataListWidget->addItem(item);
        }
    }
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
    // Python file browse button
    connect(ui->pythonFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->data->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python解释器选择", "C:\\", "exe(*.exe)");
        ui->pythonFileEdit->setText(this->data->pythonPath);
    });

    // Main file path browse button
    connect(ui->mainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->data->mainFilePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  主文件选择", "C:\\",
                                                          "Python file(*.py)");
        ui->mainPathEdit->setText(this->data->mainFilePath);
    });

    // Output file path browse button
    connect(ui->outputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->data->outputPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  输出路径", "C:\\",
                                                             QFileDialog::ShowDirsOnly);
        ui->outputPathEdit->setText(this->data->outputPath);
    });

    // Python file edit
    connect(ui->pythonFileEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->data->pythonPath = text;
    });

    // Main file path edit
    connect(ui->mainPathEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->data->mainFilePath = text;
    });

    // Output file path edit
    connect(ui->outputPathEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->data->outputPath = text;
    });

    // Output file name edit
    connect(ui->outputFileEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->data->outputFilename = text;
    });

    // Build Settings
    // Standalone
    connect(ui->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            this->data->standalone = false;
        } else if (state == Qt::Checked) {
            this->data->standalone = true;
        }
    });
    // Onefile
    connect(ui->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            this->data->onefile = false;
        } else if (state == Qt::Checked) {
            this->data->onefile = true;
        }
    });
    // Remove Output
    connect(ui->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            this->data->removeOutput = false;
        } else if (state == Qt::Checked) {
            this->data->removeOutput = true;
        }
    });

    // LTO Mode Checkbox
    // No
    connect(ui->ltoNo, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            this->data->ltoMode = LTOMode::No;
        }
    });
    // Yes
    connect(ui->ltoYes, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            this->data->ltoMode = LTOMode::Yes;
        }
    });
    // Auto
    connect(ui->ltoAuto, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            this->data->ltoMode = LTOMode::Auto;
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
        this->data->iconPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  图标路径", "C:\\",
                                                      "Icon file(*.jpg *.jpeg *.png *.ico);;All files(*)");
        ui->iconFileEdit->setText(this->data->iconPath);
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
        this->timerLabel->setText(timeString);
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
        Logger::debug(QString::number(value));
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
        switch (row) {
            case 0:
                this->data->pythonPath = item->text();
                break;
            case 1:
                this->data->mainFilePath = item->text();
                break;
            case 2:
                this->data->outputPath = item->text();
                break;
            case 3:
                this->data->outputFilename = item->text();
                break;
            case 4:
                this->data->iconPath = item->text();
                break;
            case 8:
                this->data->dataList = item->text().split(";");
                break;
            case 9:
                int ltoValue;
                switch (this->data->ltoMode) {
                    case LTOMode::Auto:
                        ltoValue = 0;
                        break;
                    case LTOMode::Yes:
                        ltoValue = 1;
                        break;
                    case LTOMode::No:
                        ltoValue = 2;
                        break;
                }
                this->ltoModeCombobox->setCurrentIndex(ltoValue);
                break;
        }
    });
    // Checkboxes
    connect(this->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        this->data->standalone = state == Qt::CheckState::Checked;
    });
    connect(this->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        this->data->onefile = state == Qt::CheckState::Checked;
    });
    connect(this->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        this->data->removeOutput = state == Qt::CheckState::Checked;
    });
    // LTO Mode Combobox
    connect(this->ltoModeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        switch (index) {
            case 0:
                this->data->ltoMode = LTOMode::Auto;
                break;
            case 1:
                this->data->ltoMode = LTOMode::Yes;
                break;
            case 2:
                this->data->ltoMode = LTOMode::No;
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
    this->timerLabel = new QLabel;
    this->timerLabel->setAlignment(Qt::AlignCenter);
    ui->statusbar->addWidget(this->timerLabel);
    ui->statusbar->addPermanentWidget(this->timerLabel, 0);
}
