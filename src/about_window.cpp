//
// Created by david on 2025/12/30.
//

// You may need to build the project (run Qt uic code generator) to get "ui_about_window.h" resolved

#include "about_window.h"
#include "ui_about_window.h"


AboutWindow::AboutWindow(QWidget *parent) :
        QWidget(parent), ui(new Ui::AboutWindow) {
    ui->setupUi(this);

    connect(ui->licensesButton, &QPushButton::clicked, this, &AboutWindow::onLicensesButtonClicked);

    ui->versionLabel->setText(QString(APP_VERSION));
    ui->authorLabel->setText(QString(APP_AUTHOR));
    ui->timeLabel->setText(QString(APP_BUILD_TIME));
}

AboutWindow::~AboutWindow() {
    delete ui;
}

void AboutWindow::onLicensesButtonClicked() {
    auto* licensesWidget = new QWidget;
    licensesWidget->setAttribute(Qt::WA_DeleteOnClose);

    licensesWidget->setFixedSize(600, 500);

    QTextEdit* textEdit = new QTextEdit(licensesWidget);
    textEdit->setReadOnly(true);
    textEdit->setFixedSize(580, 480);

    QFile fmt("../licenses/FmtLicense");
    fmt.open(QIODevice::ReadOnly);
    QString fmtLicense = QString::fromLocal8Bit(fmt.readAll());

    textEdit->append("Fmt License\n");
    textEdit->append(fmtLicense);
    textEdit->append("\n\n");

    QFile spdlog("../licenses/SpdlogLicense");;
    spdlog.open(QIODevice::ReadOnly);
    QString spdlogLicense = QString::fromLocal8Bit(spdlog.readAll());

    textEdit->append("Spdlog License\n");
    textEdit->append(spdlogLicense);

    licensesWidget->show();
}
