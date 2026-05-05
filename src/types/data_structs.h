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
#include "ElaDef.h"

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
    CustomCommand
};

using PCE = ProjectConfigEnum;

struct PackLogStruct {
    QString logFileName;
    QString logContent;
    QString logNote;

    PackLogStruct() = default;

    PackLogStruct(const QString &logFileName, const QString &logContent, const QString &logNote) {
        this->logFileName = logFileName;
        this->logContent = logContent;
        this->logNote = logNote;
    }
    PackLogStruct(const QString &logFileName, const QString &logContent) {
        this->logFileName = logFileName;
        this->logContent = logContent;
    }
};

using PackLogList = QList<PackLogStruct>;
Q_DECLARE_METATYPE(PackLogStruct)
Q_DECLARE_METATYPE(QList<PackLogStruct>)

namespace PageNS {
    Q_NAMESPACE

    enum class PageCard {
        PackPageBaseCard = 0x0000,
        PackPagePackCard = 0x0001,
        PackPageAssetCard = 0x0002,
        PackPageFileInfoCard = 0x0003,
        PackPageConsoleCard = 0x0004,

        SettingsPageGeneralCard = 0x0100,
        SettingsPageAppearanceCard = 0x0101,
        SettingsPageDefaultPathCard = 0x0102,

        NONE = -0x01
    };

    Q_ENUM_NS(PageCard)
}

using namespace PageNS;

Q_DECLARE_METATYPE(PageCard)

#endif //DATASTRUCTS_H
