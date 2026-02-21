//
// Created by redrch on 2026/1/9.
//

#ifndef UTILS_H
#define UTILS_H
#pragma once

#include <QProcess>
#include <QList>
#include <QVariant>

class Utils {
public:
    // Util functions
    static QString boolToString(bool v);
    static QString processErrorToString(QProcess::ProcessError err);
    static QString formatMilliseconds(qint64 totalMs);

    static QList<QString> listVariantToQStringList(const QList<QVariant> &list);
    static QList<QVariant> StringListToListVariant(const QList<QString> &list);
};



#endif //UTILS_H
