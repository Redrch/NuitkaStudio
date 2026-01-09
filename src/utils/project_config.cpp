//
// Created by redrch on 2026/1/7.
//

#include "project_config.h"

ProjectConfig::ProjectConfig(ProjectConfigData *data, QWidget *parent) {
    this->data = data;
    this->parent = parent;
}

ProjectConfig::~ProjectConfig() {
}

void ProjectConfig::importProject() {
    QString path = QFileDialog::getOpenFileName(this->parent, "Nuitka Studio  导入项目文件",
                                                Config::instance().getDefaultDataPath(),
                                                "Nuitka Project File(*.npf);;All files(*)");
    if (path == "") {
        return;
    }
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Open failed: " << file.errorString();
        QMessageBox::critical(this->parent, "Nuitka Studio Error", "Open failed: " + file.errorString());
        return;
    }
    QTextStream in(&file);
    in.setCodec("UTF-8");

    // Read the file
    QStringList contentList = in.readAll().split("\n");
    // Process data
    this->data->pythonPath = contentList[0].split("=")[1];
    this->data->mainFilePath = contentList[1].split("=")[1];
    this->data->outputPath = contentList[2].split("=")[1];
    this->data->outputFilename = contentList[3].split("=")[1];
    this->data->iconPath = contentList[4].split("=")[1];
    this->data->standalone = contentList[5].split("=")[1] == "true";
    this->data->onefile = contentList[6].split("=")[1] == "true";
    this->data->removeOutput = contentList[7].split("=")[1] == "true";
    // LTO
    QString ltoModeString = contentList[8].split("=")[1];
    if (ltoModeString == "Yes") {
        this->data->ltoMode = LTOMode::Yes;
    } else if (ltoModeString == "No") {
        this->data->ltoMode = LTOMode::No;
    } else if (ltoModeString == "Auto") {
        this->data->ltoMode = LTOMode::Auto;
    } else {
        QMessageBox::warning(this->parent, "Nuitka Studio Warning",
                             "LTO模式值: " + ltoModeString + " 错误，只能为Yes/No/Auto");
        return;
    }
    this->data->dataList = contentList[9].split("=")[1].split(";");

    Logger::info("导入NPF文件，参数: " + contentList.join(";"));
}

void ProjectConfig::exportProject() {
    QString path = QFileDialog::getSaveFileName(this->parent, "Nuitka Studio  导出项目文件",
                                                Config::instance().getDefaultDataPath(),
                                                "Nuitka Project File(*.npf);;All files(*)");
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Open failed: " << file.errorString();
        QMessageBox::warning(this->parent, "Nuitka Studio  Warning", "Open failed: " + file.errorString());
        return;
    }
    QTextStream out(&file);
    out.setCodec("UTF-8");

    // Write data
    // Path data
    out << "python_path=" << this->data->pythonPath << "\n";
    out << "mainfile_path=" << this->data->mainFilePath << "\n";
    out << "output_path=" << this->data->outputPath << "\n";
    out << "output_filename=" << this->data->outputFilename << "\n";
    out << "icon_path=" << this->data->iconPath << "\n";
    // Bool data
    out << "standalone=" << Utils::boolToString(this->data->standalone) << "\n";
    out << "onefile=" << Utils::boolToString(this->data->onefile) << "\n";
    out << "remove_output=" << Utils::boolToString(this->data->removeOutput) << "\n";
    // LTO data
    QString LTOModeString;
    if (this->data->ltoMode == LTOMode::Yes) {
        LTOModeString = "Yes";
    } else if (this->data->ltoMode == LTOMode::No) {
        LTOModeString = "No";
    } else if (this->data->ltoMode == LTOMode::Auto) {
        LTOModeString = "Auto";
    }
    out << "lto=" << LTOModeString << "\n";
    // Data list
    out << "data_list=" << this->data->dataList.join(";");

    file.close();

    Logger::info("导出NPF文件");
}
