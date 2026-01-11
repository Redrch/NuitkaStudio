//
// Created by redrch on 2026/1/7.
//

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H
#pragma once

enum class LTOMode {
    No,
    Yes,
    Auto
};

struct ProjectConfigData {
    QString pythonPath;
    QString mainFilePath;
    QString outputPath;
    QString outputFilename;
    QString iconPath = "";

    bool standalone = true;
    bool onefile = false;
    bool removeOutput = false;
    LTOMode ltoMode = LTOMode::Auto;
    QList<QString> dataList;

    ProjectConfigData() = default;

    ProjectConfigData(const QString & pythonPath, const QString & mainFilePath, const QString & outputPath, const QString & outputFilename) {
        this->pythonPath = pythonPath;
        this->mainFilePath = mainFilePath;
        this->outputPath = outputPath;
        this->outputFilename = outputFilename;
    }
};

#endif //DATASTRUCTS_H
