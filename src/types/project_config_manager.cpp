//
// Created by redrch on 2026/1/16.
//

#include "project_config_manager.h"
#include "simname.h"

ProjectConfigManager::ProjectConfigManager() {
    this->configList = new QList<ProjectConfigType *>();
}

ProjectConfigManager::~ProjectConfigManager() {
    for (const ProjectConfigType *config: *this->configList) {
        delete config;
    }
    delete this->configList;
};

void ProjectConfigManager::addItem(ProjectConfigType *config) const {
    if (config == nullptr) return;

    if (this->configList->contains(config)) {
        Logger::warn(QString("项目设置项%1已存在，请不要再次添加此项").arg(config->get_itemName()));
    }
    this->configList->append(config);
}

QList<ProjectConfigType *> *ProjectConfigManager::getList() const {
    return this->configList;
}

int ProjectConfigManager::getIndex(const QString &name) const {
    for (int i = 0; i < this->configList->count(); i++) {
        ProjectConfigType *config = this->configList->at(i);
        if (config->get_itemName() == name) {
            return i;
        }
    }
    return -1;
}

ProjectConfigType *ProjectConfigManager::getItem(int index) {
    return this->configList->at(index);
}

ProjectConfigType *ProjectConfigManager::getItem(PCE value) {
    int index = static_cast<int>(value);
    ProjectConfigType *config = getItem(index);
    if (config == nullptr) return nullptr;
    return config;
}

QVariant ProjectConfigManager::getItemValue(int index) {
    ProjectConfigType *config = getItem(index);
    if (config == nullptr) return QVariant();
    return config->get_itemValue();
}

QVariant ProjectConfigManager::getItemValue(PCE configValue) {
    int index = static_cast<int>(configValue);
    return getItemValue(index);
}

QString ProjectConfigManager::getItemValueToString(int index) {
    return getItemValue(index).toString();
}

QString ProjectConfigManager::getItemValueToString(PCE value) {
    return getItemValue(value).toString();
}

QStringList ProjectConfigManager::getItemValueToStringList(int index) {
    return getItemValue(index).toStringList();
}

QStringList ProjectConfigManager::getItemValueToStringList(PCE value) {
    return this->getItemValueToStringList(static_cast<int>(value));
}

bool ProjectConfigManager::getItemValueToBool(int index) {
    return getItemValue(index).toBool();
}

bool ProjectConfigManager::getItemValueToBool(PCE value) {
    return getItemValue(value).toBool();
}

int ProjectConfigManager::getLength() const {
    return this->configList->size();
}

void ProjectConfigManager::setItem(const int index, const QVariant &value) {
    ProjectConfigType *configItem = getItem(index);
    configItem->set_itemValue(value);
}

void ProjectConfigManager::setItem(PCE configValue, const QVariant &value) {
    int index = static_cast<int>(configValue);
    ProjectConfigType *configItem = getItem(index);
    configItem->set_itemValue(value);
}

void ProjectConfigManager::appendItemToStringList(int index, const QString &value) {
    QVariant rawList = this->getItemValue(index);
    QStringList stringList = rawList.toStringList();
    stringList.append(value);
    this->setItem(index, QVariant(stringList));
}

void ProjectConfigManager::appendItemToStringList(PCE configValue, const QString &value) {
    int index = static_cast<int>(configValue);
    this->appendItemToStringList(index, value);
}

void ProjectConfigManager::removeItemFromStringList(int index, const QString &value) {
    QStringList stringList = this->getItemValue(index).toStringList();
    stringList.removeOne(value);
    this->setItem(index, QVariant(stringList));
}

void ProjectConfigManager::removeItemFromStringList(int index, int valueIndex) {
    QStringList stringList = this->getItemValue(index).toStringList();
    stringList.removeAt(valueIndex);
    this->setItem(index, QVariant(stringList));
}

void ProjectConfigManager::removeItemFromStringList(PCE configValue, const QString &value) {
    this->removeItemFromStringList(static_cast<int>(configValue), value);
}

void ProjectConfigManager::removeItemFromStringList(PCE configValue, int valueIndex) {
    this->removeItemFromStringList(static_cast<int>(configValue), valueIndex);
}

void ProjectConfigManager::setList(QList<ProjectConfigType *> *list) {
    this->configList = list;
}
