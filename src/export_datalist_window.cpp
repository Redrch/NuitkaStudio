//
// Created by redrch on 2025/12/12.
//

// You may need to build the project (run Qt uic code generator) to get "ui_export_datalist_window.h" resolved

#include "export_datalist_window.h"
#include "ui_export_datalist_window.h"


ExportDataListWindow::ExportDataListWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::ExportDataListWindow) {
    ui->setupUi(this);

    // Connect Signals and slots
    connect(ui->addFileBtn, &QPushButton::clicked, this, [=]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件", "C:\\");
        ui->dataListWidget->addItem(filePath);
        this->dataList.append(filePath);
    });

    connect(ui->addDirBtn, &QPushButton::clicked, this, [=](){
        QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录", "C:\\",
                                                            QFileDialog::ShowDirsOnly);
        ui->dataListWidget->addItem(dirPath);
        this->dataList.append(dirPath);
    });

    connect(ui->removeItemBtn, &QPushButton::clicked, this, [=](){
        QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
        delete removeItem;
    });

    this->updateUI();
}

ExportDataListWindow::~ExportDataListWindow() {
    delete ui;
}

void ExportDataListWindow::setDataList(QList<QString> &dataListArg) {
    this->dataList = dataListArg;
}

void ExportDataListWindow::updateUI() {
    for (QString dataItem: this->dataList) {
        ui->dataListWidget->addItem(dataItem);
    }
}
