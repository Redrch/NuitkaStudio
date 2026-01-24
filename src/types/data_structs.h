//
// Created by redrch on 2026/1/7.
//

#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H
#pragma once

#include <QString>
#include <QMap>
#include <QList>
#include <QMetaType>

enum class LTOMode {
    No,
    Yes,
    Auto
};

Q_DECLARE_METATYPE(LTOMode);

inline QMap<int, int> configListAndUiListMap = {
    {0, 0},
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 10},
    {5, 11},
    {6, 4},
    {7, 5},
    {8, 6},
    {9, 7},
    {10, 9},
    {11, 8},
    {12, 12},
    {13, 13},
    {14, 14},
    {15, 15},
    {16, 16},
    {17, 17},
    {18, 18},
    {19, 19},
    {20, 20},
    {21, 21},
    {22, 22},
    {23, 23},
    {24, 24}
};

inline QMap<int, int> configListAndUiListInverseMap = {
    {0, 0},
    {1, 1},
    {2, 2},
    {3, 3},
    {4, 6},
    {5, 7},
    {6, 8},
    {7, 9},
    {8, 11},
    {9, 10},
    {10, 4},
    {11, 5},
    {12, 12},
    {13, 13},
    {14, 14},
    {15, 15},
    {16, 16},
    {17, 17},
    {18, 18},
    {19, 19},
    {20, 20},
    {21, 21},
    {22, 22},
    {23, 23},
    {24, 24}
};

enum class ConfigValue {
    PythonPath,
    MainfilePath,
    OutputPath,
    OutputFilename,
    ProjectPath,
    ProjectName,
    IconPath,
    Standalone,
    Onefile,
    RemoveOutput,
    LtoMode,
    DataList,
    Language,
    OriginalFilename,
    FileVersion,
    InternalName,
    Company,
    ProductName,
    ProductVersion,
    FileDescription,
    Comments,
    LegalCopyright,
    LegalTrademarks,
    PrivateBuild,
    SpecialBuild,
};

struct ProjectConfigData {
    QString pythonPath;
    QString mainFilePath;
    QString outputPath;
    QString outputFilename;
    QString projectPath;
    QString projectName;
    QString iconPath = "";

    bool standalone = true;
    bool onefile = false;
    bool removeOutput = false;
    LTOMode ltoMode = LTOMode::Auto;
    QList<QString> dataList;

    ProjectConfigData() = default;

    ProjectConfigData(const QString &pythonPath, const QString &mainFilePath, const QString &outputPath,
                      const QString &outputFilename) {
        this->pythonPath = pythonPath;
        this->mainFilePath = mainFilePath;
        this->outputPath = outputPath;
        this->outputFilename = outputFilename;
    }
};

#endif //DATASTRUCTS_H
