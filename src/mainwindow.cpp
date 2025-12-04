//
// Created by redrch on 2025/11/30.
//

#include "mainwindow.h"
#include "../ui/ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 连接信号槽
    // QStackedWidget
    connect(ui->pack_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(0);
    });
    connect(ui->settings_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(1);
    });
    connect(ui->more_btn, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentIndex(2);
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
}

MainWindow::~MainWindow() {
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
    QListWidgetItem *currentItem = ui->dataListWidget->currentItem();
    delete currentItem;
}
