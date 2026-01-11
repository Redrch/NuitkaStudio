//
// Created by redrch on 2026/1/3.
//

// You may need to build the project (run Qt uic code generator) to get "ui_new_project_window.h" resolved

#include "new_project_window.h"

#include <QFileDialog>

#include "../utils/config.h"
#include "ui_new_project_window.h"


NewProjectWindow::NewProjectWindow(QWidget *parent) : QDialog(parent), ui(new Ui::NewProjectWindow) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->setupUi(this);

    this->projectConfigData = new ProjectConfigData;

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
        ui->uvPyFilePathEdit->setText(this->pythonPath);
    });
    connect(ui->uvPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->uvPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  UV路径",
                                                    Config::instance().getDefaultMainFilePath(), "exe(*.exe)");
        ui->uvPathEdit->setText(this->uvPath);
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

ProjectConfigData *NewProjectWindow::getProjectConfigData() {
    return this->projectConfigData;
}

void NewProjectWindow::newProject() {
    ui->newProjectBtn->setEnabled(false);
    Logger::info(QString("创建项目%1在目录%2").arg(this->projectName).arg(this->projectPath));
    QString projectDirPath = QFileInfo(this->projectPath, this->projectName).absoluteFilePath();
    QDir projectDir(this->projectPath);
    projectDir.mkdir(this->projectName);

    QFile file(projectDirPath + "/main.py");
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << "#coding: utf-8" << "\n";
    out << "\n";
    out << "print('Hello,World!')" << "\n";
    file.close();

    switch (this->interpreterType) {
        case InterpreterType::Python: {
            this->projectConfigData->pythonPath = this->pythonPath;
            this->projectConfigData->mainFilePath = projectDirPath + "/main.py";
            this->projectConfigData->outputPath = projectDirPath + "/build";
            this->projectConfigData->outputFilename = projectName + ".exe";
            ProjectConfig project_config(this->projectConfigData, this);
            project_config.exportProject(projectDirPath + "/" + projectName + ".npf");
            break;
        }
        case InterpreterType::Virtualenv: {
            QProcess p;
            p.setWorkingDirectory(projectDirPath);
            p.start(this->pythonPath, QStringList() << "-m" << "venv" << ".venv");
            p.waitForFinished();

            this->projectConfigData->pythonPath = projectDirPath + "/.venv" + "/Scripts" + "/python.exe";
            this->projectConfigData->mainFilePath = projectDirPath + "/main.py";
            this->projectConfigData->outputPath = projectDirPath + "/build";
            this->projectConfigData->outputFilename = projectName + ".exe";
            ProjectConfig project_config(this->projectConfigData, this);
            project_config.exportProject(projectDirPath + "/" + projectName + ".npf");
            break;
        }

        case InterpreterType::UV: {
            QProcess p;
            p.setWorkingDirectory(projectDirPath);
            p.start(this->uvPath, QStringList() << "init" << "-p" << this->pythonPath << "--no-readme");
            p.waitForFinished();

            this->projectConfigData->pythonPath = this->pythonPath;
            this->projectConfigData->mainFilePath = projectDirPath + "/main.py";
            this->projectConfigData->outputPath = projectDirPath + "/build";
            this->projectConfigData->outputFilename = projectName + ".exe";

            ProjectConfig project_config(this->projectConfigData, this);
            project_config.exportProject(projectDirPath + "/" + projectName + ".npf");
            break;
        }
    }
    ui->newProjectBtn->setDisabled(true);

    this->accept();
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
