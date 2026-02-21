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

bool GlobalData::getBool(const QString &name) const {
    QMutexLocker lock(mutex);
    QVariant result = this->dataMap->value(name);
    return result.toBool();
}

int GlobalData::getInt(const QString &name) const {
    QMutexLocker lock(mutex);
    QVariant result = this->dataMap->value(name);
    return result.toInt();
}

double GlobalData::getDouble(const QString &name) const {
    QMutexLocker lock(mutex);
    QVariant result = this->dataMap->value(name);
    return result.toDouble();
}

void GlobalData::set(const QString &name, const QVariant &value) {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, value);
    emit this->valueChanged(name, value);
}

void GlobalData::setString(const QString &name, const QString &value) {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, QVariant(value));
    emit this->valueChanged(name, value);
}

void GlobalData::setBool(const QString &name, const bool &value) {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, QVariant(value));
    emit this->valueChanged(name, value);
}

void GlobalData::setInt(const QString &name, const int &value) {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, QVariant(value));
    emit this->valueChanged(name, value);
}

void GlobalData::setDouble(const QString &name, const double &value) {
    QMutexLocker lock(mutex);
    this->dataMap->insert(name, QVariant(value));
    emit this->valueChanged(name, value);
}
