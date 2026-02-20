//
// Created by redrch on 2026/2/15.
//

#include "global_data.h"

GlobalData::GlobalData() {
    this->mutex = new QMutex;
    this->dataMap = new QMap<QString, QVariant>;
}

GlobalData::~GlobalData() {
    delete mutex;
    delete dataMap;
}

QVariant GlobalData::get(const QString &name) const {
    QMutexLocker lock(mutex);
    QVariant result = this->dataMap->value(name);
    return result;
}

QString GlobalData::getString(const QString &name) const {
    QMutexLocker lock(mutex);
    QVariant result = this->dataMap->value(name);
    return result.toString();
}

void GlobalData::set(const QString &name, const QVariant &value) const {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, value);
}

void GlobalData::setString(const QString &name, const QString &value) const {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, QVariant(value));
}


