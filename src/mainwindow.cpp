//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    if (!QFile::exists(Config::instance().getConfigPath())) {
        Config::instance().writeConfig();
    }
    Config::instance().readConfig();

//    this->iconPath = QString("");
//    this->standalone = true;
//    this->onefile = true;
//    this->ltoMode = LTOMode::Yes;
//
//    // Debug Code
//    this->pythonPath = "D:/Develop/Python/RandomText/.venv/Scripts/python.exe";
//    this->mainFilePath = "D:/Develop/Python/RandomText/main.py";
//    this->outputPath = "D:/Develop/Python/RandomText";
//    this->outputFilename = "RandomText.exe";
//    this->dataList = {"D:/Develop/Python/RandomText/a", "D:/Develop/Python/RandomText/b",
//                      "D:/Develop/Python/RandomText/c"};


    // Connect signal and slot
    // QStackedWidget
    connect(ui->pack_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
        Config::instance().writeConfig();
    });
    connect(ui->settings_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
        Config::instance().writeConfig();
    });
    connect(ui->export_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2);
        Config::instance().writeConfig();
    });

    // Python file browse button
    connect(ui->pythonFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python解释器选择", "C:\\", "exe(*.exe)");
        ui->pythonFileEdit->setText(this->pythonPath);
    });

    // Main file path browse button
    connect(ui->mainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->mainFilePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  主文件选择", "C:\\",
                                                          "Python file(*.py)");
        ui->mainPathEdit->setText(this->mainFilePath);
    });

    // Output file path browse button
    connect(ui->outputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->outputPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  输出路径", "C:\\",
                                                             QFileDialog::ShowDirsOnly);
        ui->outputPathEdit->setText(this->outputPath);
    });

    // Build Settings
    // Standalone
    connect(ui->standaloneCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            this->standalone = false;
        } else if (state == Qt::Checked) {
            this->standalone = true;
        }
    });
    // Onefile
    connect(ui->onefileCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            this->onefile = false;
        } else if (state == Qt::Checked) {
            this->onefile = true;
        }
    });
    // Remove Output
    connect(ui->removeOutputCheckbox, &QCheckBox::stateChanged, this, [=](int state) {
        if (state == Qt::Unchecked) {
            this->removeOutput = false;
        } else if (state == Qt::Checked) {
            this->removeOutput = true;
        }
    });

    // LTO Mode Checkbox
    // No
    connect(ui->ltoNo, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            this->ltoMode = LTOMode::No;
        }
    });
    // Yes
    connect(ui->ltoYes, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            this->ltoMode = LTOMode::Yes;
        }
    });
    // Auto
    connect(ui->ltoAuto, &QCheckBox::stateChanged, this, [=]() {
        if (ui->ltoNo->checkState() == Qt::CheckState::Checked) {
            this->ltoMode = LTOMode::Auto;
        }
    });

    // Modify Data List
    // Add file button
    connect(ui->addFileBtn, &QPushButton::clicked, this, &MainWindow::on_AddDataFileItem_clicked);
    // Add dir button
    connect(ui->addDirBtn, &QPushButton::clicked, this, &MainWindow::on_AddDataDirItem_clicked);
    // Remove item button
    connect(ui->removeItemBtn, &QPushButton::clicked, this, &MainWindow::on_RemoveItem_clicked);

    // Icon browse
    connect(ui->iconFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->iconPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  图标路径", "C:\\",
                                                      "Icon file(*.jpg *.jpeg *.png *.ico);;All files(*)");
        ui->iconFileEdit->setText(this->iconPath);
    });

    // Start pack
    connect(ui->startPackBtn, &QPushButton::clicked, this, &MainWindow::startPack);
    // Clear Console Edit
    connect(ui->clearConsoleBtn, &QPushButton::clicked, this, [=]() {
        ui->consoleOutputEdit->clear();
    });
    // Import button
    connect(ui->importBtn, &QPushButton::clicked, this, &MainWindow::importProject);


    // Settings
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

    // Export
    // Export Button
    connect(ui->exportBtn, &QPushButton::clicked, this, &MainWindow::exportProject);
    // Cell Double-Clicked
    connect(ui->projectTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::on_ProjectTable_cellDoubleClicked);
    // Item Changed
    connect(ui->projectTable, &QTableWidget::itemChanged, this, [=](QTableWidgetItem *item) {
        int row = item->row();
        switch (row) {
            case 0:
                this->pythonPath = item->text();
                break;
            case 1:
                this->mainFilePath = item->text();
                break;
            case 2:
                this->outputPath = item->text();
                break;
            case 3:
                this->outputFilename = item->text();
                break;
            case 4:
                this->iconPath = item->text();
                break;
            case 5:
                this->standalone = item->text() == "true";
                break;
            case 6:
                this->onefile = item->text() == "true";
                break;
            case 7:
                this->dataList = item->text().split(";");
                break;
            case 8:
                QString text = item->text();
                if (text == "Yes") {
                    this->ltoMode = LTOMode::Yes;
                } else if (text == "No") {
                    this->ltoMode = LTOMode::No;
                } else if (text == "Auto") {
                    this->ltoMode = LTOMode::Auto;
                } else {
                    QMessageBox::warning(this, "Nuitka Studio Warning",
                                         "LTO模式值: " + text + " 错误，只能为Yes/No/Auto");
                    return;
                }
                break;
        }
    });


    // Init UI
    // Export
    // 自适应行宽/行高
    ui->projectTable->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->projectTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    this->updateUI();
}

MainWindow::~MainWindow() {
    delete ui;
}

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
}

void MainWindow::startPack() {
    ui->consoleOutputEdit->append(QString("-------------- 开始打包 %1 -------------").arg(
            QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz")));
    QProcess *proc = new QProcess(this);

    // Signals and slots
    // 合并普通和错误输出
    proc->setProcessChannelMode(QProcess::MergedChannels);
    // output
    connect(proc, &QProcess::readyReadStandardOutput, this, [=]() {
        QString out = QString::fromLocal8Bit(proc->readAllStandardOutput());
        ui->consoleOutputEdit->append(out);
    });
    // finished
    connect(proc, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus exitStatus) {
                ui->consoleOutputEdit->append("-------------- 打包结束 -------------");
                proc->deleteLater();
            });
    // error occurred
    connect(proc, &QProcess::errorOccurred, this, [=](QProcess::ProcessError error) {
        qWarning() << "command error: " << error;
        ui->consoleOutputEdit->append("Error: " + this->processErrorToString(error));
    });

    // build args
    QStringList args = QStringList();
    args << "-m" << "nuitka";
    if (this->standalone) {
        args << "--standalone";
    }
    if (this->onefile) {
        args << "--onefile";
    }
    if (this->removeOutput) {
        args << "--remove-output";
    }

    // LTO
    switch (this->ltoMode) {
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

    args << mainFilePath;
    args << "--output-dir=" + this->outputPath;
    args << "--output-filename=" + this->outputFilename;

    if (this->iconPath != QString("")) {
        args << "--windows-icon=" + this->iconPath;
    }

    proc->start(this->pythonPath, args);
    ui->consoleOutputEdit->append(this->pythonPath + " " + args.join(" "));
}

void MainWindow::importProject() {
    QString path = QFileDialog::getOpenFileName(this, "Nuitka Studio  导入项目文件",
                                                Config::instance().getDefaultDataPath(),
                                                "Nuitka Project File(*.npf);;All files(*)");
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Open failed: " << file.errorString();
        QMessageBox::critical(this, "Nuitka Studio Error", "Open failed: " + file.errorString());
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");

    // Read the file
    QStringList contentList = in.readAll().split("\n");
    // Process data
    this->pythonPath = contentList[0].split("=")[1];
    this->mainFilePath = contentList[1].split("=")[1];
    this->outputPath = contentList[2].split("=")[1];
    this->outputFilename = contentList[3].split("=")[1];
    this->iconPath = contentList[4].split("=")[1];
    this->standalone = contentList[5].split("=")[1] == "true";
    this->onefile = contentList[6].split("=")[1] == "true";
    this->removeOutput = contentList[7].split("=")[1] == "true";
    // LTO
    QString ltoModeString = contentList[8].split("=")[1];
    if (ltoModeString == "Yes") {
        this->ltoMode = LTOMode::Yes;
    } else if (ltoModeString == "No") {
        this->ltoMode = LTOMode::No;
    } else if (ltoModeString == "Auto") {
        this->ltoMode = LTOMode::Auto;
    } else {
        QMessageBox::warning(this, "Nuitka Studio Warning",
                             "LTO模式值: " + ltoModeString + " 错误，只能为Yes/No/Auto");
        return;
    }
    this->dataList = contentList[9].split("=")[1].split(";");

    // Update UI
    this->updateUI();
}

void MainWindow::exportProject() {
    QString path = QFileDialog::getSaveFileName(this, "Nuitka Studio  导出项目文件",
                                                Config::instance().getDefaultDataPath(),
                                                "Nuitka Project File(*.npf);;All files(*)");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Open failed: " << file.errorString();
        QMessageBox::warning(this, "Nuitka Studio  Warning", "Open failed: " + file.errorString());
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    // Write data
    // Path data
    out << "python_path=" << this->pythonPath << "\n";
    out << "mainfile_path=" << this->mainFilePath << "\n";
    out << "output_path=" << this->outputPath << "\n";
    out << "output_filename=" << this->outputFilename << "\n";
    out << "icon_path=" << this->iconPath << "\n";
    // Bool data
    out << "standalone=" << MainWindow::boolToString(this->standalone) << "\n";
    out << "onefile=" << MainWindow::boolToString(this->onefile) << "\n";
    out << "remove_output=" << MainWindow::boolToString(this->removeOutput) << "\n";
    // LTO data
    QString LTOModeString;
    if (this->ltoMode == LTOMode::Yes) {
        LTOModeString = "Yes";
    } else if (this->ltoMode == LTOMode::No) {
        LTOModeString = "No";
    } else if (this->ltoMode == LTOMode::Auto) {
        LTOModeString = "Auto";
    }
    out << "lto=" << LTOModeString << "\n";
    // Data list
    out << "data_list=" << this->dataList.join(";");

    file.close();
}

void MainWindow::on_AddDataFileItem_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件",
                                                    Config::instance().getDefaultDataPath());
    ui->dataListWidget->addItem(filePath);
    this->dataList.append(filePath);
}

void MainWindow::on_AddDataDirItem_clicked() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                        Config::instance().getDefaultDataPath(),
                                                        QFileDialog::ShowDirsOnly);
    ui->dataListWidget->addItem(dirPath);
    this->dataList.append(dirPath);
}

void MainWindow::on_RemoveItem_clicked() {
    QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
    delete removeItem;
}

void MainWindow::on_ProjectTable_cellDoubleClicked(int row, int column) {
    // Data List
    if (row == 7 and column == 1) {
        auto *dataListWindow = new ExportDataListWindow();
        dataListWindow->setAttribute(Qt::WA_DeleteOnClose);
        dataListWindow->setDataList(this->dataList);

        connect(dataListWindow, &ExportDataListWindow::dataListChanged, this, [=](const QList<QString> &newDataList) {
            this->dataList = newDataList;
            this->updateUI();
        });

        dataListWindow->updateUI();
        dataListWindow->show();
    }
}

QString MainWindow::processErrorToString(QProcess::ProcessError err) {
    switch (err) {
        case QProcess::FailedToStart:
            return QStringLiteral("FailedToStart");
        case QProcess::Crashed:
            return QStringLiteral("Crashed");
        case QProcess::Timedout:
            return QStringLiteral("Timedout");
        case QProcess::ReadError:
            return QStringLiteral("ReadError");
        case QProcess::WriteError:
            return QStringLiteral("WriteError");
        case QProcess::UnknownError:
            return QStringLiteral("UnknownError");
    }
    return QStringLiteral("UnknownProcessError: ") + QString::number(static_cast<int>(err));
}

void MainWindow::updateExportTable() {
    ui->projectTable->setItem(0, 1, new QTableWidgetItem(this->pythonPath));
    ui->projectTable->setItem(1, 1, new QTableWidgetItem(this->mainFilePath));
    ui->projectTable->setItem(2, 1, new QTableWidgetItem(this->outputPath));
    ui->projectTable->setItem(3, 1, new QTableWidgetItem(this->outputFilename));
    ui->projectTable->setItem(4, 1, new QTableWidgetItem(this->iconPath));
    ui->projectTable->setItem(5, 1, new QTableWidgetItem(MainWindow::boolToString(this->standalone)));
    ui->projectTable->setItem(6, 1, new QTableWidgetItem(MainWindow::boolToString(this->onefile)));
    ui->projectTable->setItem(7, 1, new QTableWidgetItem(this->dataList.join(";")));
    QString LTOModeString;
    if (this->ltoMode == LTOMode::Yes) {
        LTOModeString = "Yes";
    } else if (this->ltoMode == LTOMode::No) {
        LTOModeString = "No";
    } else if (this->ltoMode == LTOMode::Auto) {
        LTOModeString = "Auto";
    }
    ui->projectTable->setItem(8, 1, new QTableWidgetItem(LTOModeString));
}

void MainWindow::updatePackUI() {
    ui->pythonFileEdit->setText(this->pythonPath);
    ui->mainPathEdit->setText(this->mainFilePath);
    ui->outputPathEdit->setText(this->outputPath);
    ui->outputFileEdit->setText(this->outputFilename);
    ui->iconFileEdit->setText(this->iconPath);
    ui->standaloneCheckbox->setCheckState(this->standalone ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->onefileCheckbox->setCheckState(this->onefile ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->removeOutputCheckbox->setCheckState(this->removeOutput ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    // LTO
    switch (this->ltoMode) {
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
    for (QString item: this->dataList) {
        ui->dataListWidget->addItem(item);
    }
}

QString MainWindow::boolToString(bool v) {
    return (v ? "true" : "false");
}
