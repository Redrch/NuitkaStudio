//
// Created by redrch on 2026/1/7.
//

#pragma once
#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <QString>
#include <QMap>
#include <QList>
#include <QMetaType>
#include <QVariant>
#include "constants.h"

enum class LTOMode {
    Auto,
    Yes,
    No
};

Q_DECLARE_METATYPE(LTOMode);

inline QDataStream &operator<<(QDataStream &out, const LTOMode &mode) {
    // 使用 QVariant 明确选择 QVariant 的序列化重载，避免与其他重载冲突
    out << QVariant(static_cast<int>(mode));
    return out;
}

inline QDataStream &operator>>(QDataStream &in, LTOMode &mode) {
    QVariant v;
    in >> v;
    mode = static_cast<LTOMode>(v.toInt());
    return in;
}

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
};

enum class ProjectConfigEnum {
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
    FileVersion,
    Company,
    ProductName,
    ProductVersion,
    FileDescription,
    LegalCopyright,
    LegalTrademarks,
};

using PCE = ProjectConfigEnum;

struct PackLog {
    QString logFileName;
    QString logContent;
    QString logNote;

    PackLog() = default;

    PackLog(const QString &logFileName, const QString &logContent, const QString &logNote) {
        this->logFileName = logFileName;
        this->logContent = logContent;
        this->logNote = logNote;
    }
    PackLog(const QString &logFileName, const QString &logContent) {
        this->logFileName = logFileName;
        this->logContent = logContent;
    }
};


#endif //DATASTRUCTS_H
