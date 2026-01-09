//
// Created by redrch on 2026/1/9.
//

#ifndef UTILS_H
#define UTILS_H
#pragma once

#include <QProcess>

class Utils {
public:
    // Util functions
    static QString boolToString(bool v);
    static QString processErrorToString(QProcess::ProcessError err);
    static QString formatMilliseconds(qint64 totalMs);
};



#endif //UTILS_H
