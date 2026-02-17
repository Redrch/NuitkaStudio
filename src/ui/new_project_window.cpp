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
                                                              config.getDefaultMainFilePath());
        ui->projectPathEdit->setText(this->projectPath);
    });
    connect(ui->pyFileBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python路径",
                                                        config.getDefaultPythonPath(), "exe(*.exe)");
        ui->pyFileEdit->setText(this->pythonPath);
    });
    connect(ui->venvPyFilePathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python路径",
                                                        config.getDefaultPythonPath(), "exe(*.exe)");
        ui->venvPyFilePathEdit->setText(this->pythonPath);
    });
    connect(ui->uvPyFilePathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->pythonPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  Python路径",
                                                        config.getDefaultPythonPath(), "exe(*.exe)");
        ui->uvPyFilePathEdit->setText(this->pythonPath);
    });
    connect(ui->uvPathBrowseBtn, &QPushButton::clicked, this, [=]() {
        this->uvPath = QFileDialog::getOpenFileName(this, "Nuitka Studio  UV路径",
                                                    config.getDefaultMainFilePath(), "exe(*.exe)");
        ui->uvPathEdit->setText(this->uvPath);
    });

    // edit
    connect(ui->projectPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->projectPath = text;
    });
    connect(ui->projectNameEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->projectName = text;
    });
    connect(ui->pyFileEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->pythonPath = text;
    });
    connect(ui->venvPyFilePathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->pythonPath = text;
    });
    connect(ui->uvPyFilePathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->pythonPath = text;
    });
    connect(ui->uvPathEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        this->uvPath = text;
    });
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
            PCM.setItem(PCE::PythonPath, this->pythonPath);
            break;
        }
        case InterpreterType::Virtualenv: {
            QProcess p;
            p.setWorkingDirectory(projectDirPath);
            p.start(this->pythonPath, QStringList() << "-m" << "venv" << ".venv");
            p.waitForFinished();

            PCM.setItem(PCE::PythonPath,
                                                     projectDirPath + "/.venv" + "/Scripts" + "/python.exe");
            break;
        }
        case InterpreterType::UV: {
            QProcess p;
            p.setWorkingDirectory(projectDirPath);
            p.start(this->uvPath, QStringList() << "init" << "-p" << this->pythonPath << "--no-readme");
            p.waitForFinished();
            p.start(this->uvPath, QStringList() << "venv");
            p.waitForFinished();

            PCM.setItem(PCE::PythonPath,
                                                     projectDirPath + "/.venv" + "/Scripts" + "/python.exe");
            break;
        }
    }

    PCM.setItem(PCE::MainfilePath, projectDirPath + "/main.py");
    PCM.setItem(PCE::OutputPath, projectDirPath + "/build");
    PCM.setItem(PCE::OutputFilename, this->projectName + ".exe");
    PCM.setItem(PCE::ProjectPath, projectDirPath);
    PCM.setItem(PCE::ProjectName, this->projectName);
    PCM.setItem(PCE::FileVersion, "1.0.0.0");
    PCM.setItem(PCE::ProductVersion, "1.0.0.0");

    ProjectConfig project_config(this);
    project_config.exportProject(projectDirPath + "/" + projectName + ".npf");
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

void NewProjectWindow::installNuitka(QProcess *process) {
    QString projectDirPath = QFileInfo(this->projectPath, this->projectName).absoluteFilePath();
    switch (this->interpreterType) {
        case InterpreterType::Python: {
            process->setWorkingDirectory(projectDirPath);
            // find nuitka
            process->start(PCM.getItemValueToString(PCE::PythonPath),
                           QStringList() << "-m" << "pip" << "list");
            process->waitForFinished();
            QString out = QString::fromLocal8Bit(process->readAllStandardOutput());

            if (!out.contains("nuitka")) {
                // install nuitka
                process->start(PCM.getItemValueToString(PCE::PythonPath),
                               QStringList() << "-m" << "pip" << "install" << "nuitka");
            }
            break;
        }
        case InterpreterType::Virtualenv: {
            process->setWorkingDirectory(projectDirPath);
            // find nuitka
            process->start(PCM.getItemValueToString(PCE::PythonPath),
                           QStringList() << "-m" << "pip" << "list");
            process->waitForFinished();
            QString out = QString::fromLocal8Bit(process->readAllStandardOutput());

            if (!out.contains("nuitka")) {
                // install nuitka
                process->start(PCM.getItemValueToString(PCE::PythonPath),
                               QStringList() << "-m" << "pip" << "install" << "nuitka");
            }
            break;
        }
        case InterpreterType::UV: {
            process->setWorkingDirectory(projectDirPath);
            // find nuitka
            process->start(this->uvPath, QStringList() << "pip" << "list");
            process->waitForFinished();
            QString out = QString::fromLocal8Bit(process->readAllStandardOutput());

            if (!out.contains("nuitka")) {
                // install nuitka
                process->start(this->uvPath, QStringList() << "add" << "nuitka");
            }
            break;
        }
    }
}
