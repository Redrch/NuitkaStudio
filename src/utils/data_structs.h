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
    QString iconPath;

    bool standalone = true;
    bool onefile = false;
    bool removeOutput = false;
    LTOMode ltoMode = LTOMode::Auto;
    QList<QString> dataList;
};

#endif //DATASTRUCTS_H
