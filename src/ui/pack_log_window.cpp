//
// Created by redrch on 2026/2/18.
//
#include "pack_log_window.h"
#include "ui_pack_log_window.h"


PackLogWindow::PackLogWindow(QWidget *parent) : QWidget(parent), ui(new Ui::PackLogWindow) {
    ui->setupUi(this);

    // connect
    connect(ui->logList, QListWidget::currentItemChanged, this,
            [=](QListWidgetItem *current, QListWidgetItem *previous) {
                if (current) {
                    ui->logText->setText(this->log.value(current->text()));
                }
            });
}

PackLogWindow::~PackLogWindow() {
    delete ui;
}

void PackLogWindow::setLog(const QMap<QString, QString> &log) {
    this->log = log;
    for (const QString &key : log.keys()) {
        ui->logList->addItem(key);
    }
}

