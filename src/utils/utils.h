//
// Created by redrch on 2026/1/9.
//

#ifndef UTILS_H
#define UTILS_H
#pragma once

#include <QProcess>
#include <QList>
#include <QVariant>
#include <QMetaEnum>
#include "logger.h"

class Utils {
public:
    // Util functions
    static QString boolToString(bool v);

    static QString processErrorToString(QProcess::ProcessError err);

    static QString formatMilliseconds(qint64 totalMs);

    static QList<QString> listVariantToQStringList(const QList<QVariant> &list);

    static QList<QVariant> StringListToListVariant(const QList<QString> &list);

    /**
    *
    * @tparam T The enum type must be declared in Qt, and it also has a NONE type.
    */
    template<typename T>
    static QString enumToString(T enumValue) {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        QString string = QString::fromUtf8(metaEnum.valueToKey(static_cast<int>(enumValue)));
        return string;
    }

    /**
     *
     * @tparam T The enum type must be declared in Qt, and it also has a NONE type.
     */
    template<typename T>
    static T stringToEnum(const QString &string) {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();
        int value = metaEnum.keyToValue(string.toUtf8().constData());
        if (value == -1) {
            Logger::warn(QString("值%1在枚举类型%2中不存在").arg(string).arg(metaEnum.enumName()));
            return T::NONE;
        }

        return static_cast<T>(value);
    }

    /**
    *
    * @tparam T The enum type must be declared in Qt, and it also has a NONE type.
    */
    template<typename T>
    static T intToEnum(int value) {
        QMetaEnum metaEnum = QMetaEnum::fromType<T>();

        // 2. 检查该 int 值是否在枚举中有对应的 Key
        if (metaEnum.valueToKey(value) != nullptr) {
            // 合法值，进行转换
            T status = static_cast<T>(value);
            return status;
        }
        // 非法值，进行拦截
        Logger::error(QString("枚举%1中没有值%2").arg(metaEnum.enumName()).arg(value));
        return T::NONE;
    }

    template<typename T>
    static int enumToInt(T enumValue) {
        return static_cast<int>(enumValue);
    }
};


#endif //UTILS_H
