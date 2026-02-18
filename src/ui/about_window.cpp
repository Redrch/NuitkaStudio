//
// Created by redrch on 2025/12/30.
//

// You may need to build the project (run Qt uic code generator) to get "ui_about_window.h" resolved

#include "about_window.h"
#include "ui_about_window.h"


AboutWindow::AboutWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::AboutWindow) {
    ui->setupUi(this);

    ui->versionLabel->setText(QString(APP_VERSION));
    ui->authorLabel->setText(QString(APP_AUTHOR));
    ui->timeLabel->setText(QString(APP_BUILD_TIME));
}

AboutWindow::~AboutWindow() {
    delete ui;
}
