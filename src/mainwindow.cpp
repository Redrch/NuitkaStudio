//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    this->config = new Config;
    if (!QFile::exists(this->config->getConfigPath())) {
        this->config->writeConfig();
    }
    this->config->readConfig();


    // Connect signal and slot
    // QStackedWidget
    connect(ui->pack_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
        this->config->writeConfig();
    });
    connect(ui->settings_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
        this->config->writeConfig();
    });
    connect(ui->export_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2);
        this->config->writeConfig();
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


    // Settings
    // General Settings
    // Console Input Encoding
    connect(ui->consoleInputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = this->config->encodingEnumFromInt(index);
                this->config->setConsoleInputEncoding(encoding);
            });
    // Console Output Encoding
    connect(ui->consoleOutputEncodingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                EncodingEnum encoding = this->config->encodingEnumFromInt(index);
                this->config->setConsoleOutputEncoding(encoding);
            });


    // Default Path Settings
    // Browse Buttons
    // Default Python Path Browse
    connect(ui->defaultPyPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->config->setDefaultPythonPath(
                QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认Python解释器路径选择",
                                                  this->config->getDefaultPythonPath(),
                                                  QFileDialog::ShowDirsOnly));
        ui->defaultPyPathEdit->setText(this->config->getDefaultPythonPath());
    });
    // Default Main File Path Browse
    connect(ui->defaultMainPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->config->setDefaultMainFilePath(
                QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认主文件路径选择",
                                                  this->config->getDefaultMainFilePath(),
                                                  QFileDialog::ShowDirsOnly));
        ui->defaultMainPathEdit->setText(this->config->getDefaultMainFilePath());
    });
    // Default Output Path Browse
    connect(ui->defaultOutputPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->config->setDefaultOutputPath(
                QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认输出路径选择",
                                                  this->config->getDefaultIconPath(),
                                                  QFileDialog::ShowDirsOnly));
        ui->defaultOutputPathEdit->setText(this->config->getDefaultOutputPath());
    });
    // Default Icon Path Browse
    connect(ui->defaultIconPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->config->setDefaultIconPath(
                QFileDialog::getExistingDirectory(this, "Nuitka Studio  默认图标路径选择",
                                                  this->config->getDefaultIconPath(),
                                                  QFileDialog::ShowDirsOnly));
        ui->defaultIconPathEdit->setText(this->config->getDefaultIconPath());
    });

    // Line Edits
    // Python Edit
    connect(ui->defaultPyPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->config->setDefaultPythonPath(text);
    });
    // Main file Edit
    connect(ui->defaultMainPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->config->setDefaultMainFilePath(text);
    });
    // Output Edit
    connect(ui->defaultOutputPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->config->setDefaultOutputPath(text);
    });
    // Icon Edit
    connect(ui->defaultIconPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->config->setDefaultIconPath(text);
    });

    // Save button
    connect(ui->saveSettingsBtn, &QPushButton::clicked, this, [=]() {
        this->config->writeConfig();
    });


    // Init UI
    // Settings
    ui->defaultPyPathEdit->setText(this->config->getDefaultPythonPath());
    ui->defaultMainPathEdit->setText(this->config->getDefaultMainFilePath());
    ui->defaultOutputPathEdit->setText(this->config->getDefaultOutputPath());
    ui->defaultIconPathEdit->setText(this->config->getDefaultIconPath());

    ui->consoleInputEncodingCombo->setCurrentIndex(
            this->config->encodingEnumToInt(this->config->getConsoleInputEncoding()));
    ui->consoleOutputEncodingCombo->setCurrentIndex(
            this->config->encodingEnumToInt(this->config->getConsoleOutputEncoding()));
}

MainWindow::~MainWindow() {
    delete this->config;
    delete ui;
}

void MainWindow::startPack() {

}

void MainWindow::on_AddDataFileItem_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件", "C:\\");
    ui->dataListWidget->addItem(filePath);
    this->dataList.append(filePath);
}

void MainWindow::on_AddDataDirItem_clicked() {
    QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录", "C:\\",
                                                        QFileDialog::ShowDirsOnly);
    ui->dataListWidget->addItem(dirPath);
    this->dataList.append(dirPath);
}

void MainWindow::on_RemoveItem_clicked() {
    QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
    delete removeItem;
}
