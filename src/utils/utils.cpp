//
// Created by redrch on 2026/1/9.
//

#include "utils.h"


QString Utils::boolToString(bool v) {
    return (v ? "true" : "false");
}

QString Utils::processErrorToString(QProcess::ProcessError err) {
    switch (err) {
        case QProcess::FailedToStart:
            return QStringLiteral("FailedToStart");
        case QProcess::Crashed:
            return QStringLiteral("Crashed");
        case QProcess::Timedout:
            return QStringLiteral("Timedout");
        case QProcess::ReadError:
            return QStringLiteral("ReadError");
        case QProcess::WriteError:
            return QStringLiteral("WriteError");
        case QProcess::UnknownError:
            return QStringLiteral("UnknownError");
    }
    return QStringLiteral("UnknownProcessError: ") + QString::number(static_cast<int>(err));
}

QString Utils::formatMilliseconds(qint64 totalMs) {
    bool neg = totalMs < 0;
    if (neg) totalMs = -totalMs;
    qint64 h = totalMs / 3600000;
    qint64 m = (totalMs % 3600000) / 60000;
    qint64 s = (totalMs % 60000) / 1000;
    qint64 ms = totalMs % 1000;
    return QString("%1%2:%3:%4:%5")
            .arg(neg ? "-" : "")
            .arg(h, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'))
            .arg(ms, 3, 10, QChar('0'));
}
