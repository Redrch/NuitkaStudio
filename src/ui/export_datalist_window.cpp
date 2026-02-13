//
// Created by redrch on 2025/12/12.
//

// You may need to build the project (run Qt uic code generator) to get "ui_export_datalist_window.h" resolved

#include "export_datalist_window.h"
#include "ui_export_datalist_window.h"


ExportDataListWindow::ExportDataListWindow(QWidget *parent) : QWidget(parent), ui(new Ui::ExportDataListWindow) {
    ui->setupUi(this);

    // Connect Signals and slots
    connect(ui->addFileBtn, &QPushButton::clicked, this, [=]() {
        QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio  数据文件",
                                                        Config::instance().getDefaultDataPath());
        if (filePath == "") {
            return;
        }

        ui->dataListWidget->addItem(filePath);
        PCM::instance().appendItemToStringList(ConfigValue::DataList, filePath);
    });

    connect(ui->addDirBtn, &QPushButton::clicked, this, [=]() {
        QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                            Config::instance().getDefaultDataPath(),
                                                            QFileDialog::ShowDirsOnly);
        if (dirPath == "") {
            return;
        }

        ui->dataListWidget->addItem(dirPath);
        PCM::instance().appendItemToStringList(ConfigValue::DataList, dirPath);
    });

    connect(ui->removeItemBtn, &QPushButton::clicked, this, [=]() {
        QListWidgetItem *removeItem = ui->dataListWidget->takeItem(ui->dataListWidget->currentRow());
        if (removeItem == nullptr) {
            return;
        }
        delete removeItem;
    });

    this->updateUI();
}

ExportDataListWindow::~ExportDataListWindow() {
    delete ui;
}

void ExportDataListWindow::updateUI() {
    ui->dataListWidget->clear();
    for (const QString &dataItem: PCM::instance().getItemValue(ConfigValue::DataList).toStringList()) {
        if (dataItem.isEmpty()) {
            continue;
        }
        ui->dataListWidget->addItem(dataItem);
    }
}
