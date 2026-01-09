//
// Created by redrch on 2026/1/3.
//

// You may need to build the project (run Qt uic code generator) to get "ui_new_project_window.h" resolved

#include "new_project_window.h"

#include <QFileDialog>

#include "../utils/config.h"
#include "ui_new_project_window.h"


NewProjectWindow::NewProjectWindow(QWidget *parent) : QWidget(parent), ui(new Ui::NewProjectWindow) {
    ui->setupUi(this);

    this->connectPath();
    connect(ui->pyTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {
        this->onPyTypeComboBoxCurrentIndexChanged(index);
    });
    connect(ui->newProjectBtn, &QPushButton::clicked, this, &NewProjectWindow::newProject);
}

NewProjectWindow::~NewProjectWindow() {
    delete ui;
}

void NewProjectWindow::connectPath() {
    // button
    connect(ui->projectPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->projectPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  项目路径",
                                                              Config::instance().getDefaultMainFilePath());
        ui->projectPathEdit->setText(this->projectPath);
    });
    connect(ui->pyFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python路径",
                                                        Config::instance().getDefaultPythonPath(), "exe(*.exe)");
        ui->pyFileEdit->setText(this->pythonPath);
    });
    connect(ui->venvPyFilePathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python路径",
                                                        Config::instance().getDefaultPythonPath(), "exe(*.exe)");
        ui->venvPyFilePathEdit->setText(this->pythonPath);
    });
    connect(ui->uvPyFilePathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python路径",
                                                        Config::instance().getDefaultPythonPath(), "exe(*.exe)");
    });
    connect(ui->uvPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->uvPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  UV路径",
                                                    Config::instance().getDefaultMainFilePath(), "exe(*.exe)");
    });

    // edit
    connect(ui->projectPathEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->projectPath = text;
    });
    connect(ui->projectNameEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->projectName = text;
    });
    connect(ui->pyFileEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->pythonPath = text;
    });
    connect(ui->venvPyFilePathEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->pythonPath = text;
    });
    connect(ui->uvPyFilePathEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->pythonPath = text;
    });
    connect(ui->uvPathEdit, &QLineEdit::textChanged, this, [=](QString text) {
        this->uvPath = text;
    });
}

void NewProjectWindow::newProject() {
    Logger::info(QString("创建项目%1在目录%2").arg(this->projectName).arg(this->projectPath));
    QString projectDirPath = QFileInfo(this->projectPath, this->projectName).absoluteFilePath();
    QDir projectDir(this->projectPath);
    projectDir.mkdir(this->projectName);
    switch (this->interpreterType) {
        case InterpreterType::Python:

            break;
        case InterpreterType::Virtualenv:
            break;
        case InterpreterType::UV:
            break;
    }
}

void NewProjectWindow::onPyTypeComboBoxCurrentIndexChanged(int index) {
    switch (index) {
        case 0:
            this->interpreterType = InterpreterType::Python;
            break;
        case 1:
            this->interpreterType = InterpreterType::Virtualenv;
            break;
        case 2:
            this->interpreterType = InterpreterType::UV;
            break;
        default:
            QMessageBox::warning(this, "Nuitka Studio Warning", QString("没有这种解释器类型: %1").arg(index));
    }

    ui->projectPyPathStackedWidget->setCurrentIndex(index);
}
