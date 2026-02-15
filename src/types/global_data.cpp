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

void GlobalData::set(const QString &name, const QVariant &value) const {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, value);
}

