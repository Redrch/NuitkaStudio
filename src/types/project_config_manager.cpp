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

ProjectConfigType *ProjectConfigManager::getItem(int index) const {
    if (index < 0 || index >= this->configList->size()) {
        Logger::error("Crash prevented! Out of range index:" + index);
        return nullptr;
    }
    return this->configList->at(index);
}

ProjectConfigType *ProjectConfigManager::getItem(PCE value) {
    int index = static_cast<int>(value);
    ProjectConfigType *config = getItem(index);
    if (config == nullptr) return nullptr;
    return config;
}

QVariant ProjectConfigManager::get(int index) {
    ProjectConfigType *config = getItem(index);
    if (config == nullptr) return QVariant();
    return config->get_itemValue();
}

QVariant ProjectConfigManager::get(PCE configValue) {
    int index = static_cast<int>(configValue);
    return get(index);
}

QString ProjectConfigManager::getString(int index) {
    return get(index).toString();
}

QString ProjectConfigManager::getString(PCE value) {
    return get(value).toString();
}

QStringList ProjectConfigManager::getStringList(int index) {
    return get(index).toStringList();
}

QStringList ProjectConfigManager::getStringList(PCE value) {
    return this->getStringList(static_cast<int>(value));
}

bool ProjectConfigManager::getBool(int index) {
    return get(index).toBool();
}

bool ProjectConfigManager::getBool(PCE value) {
    return get(value).toBool();
}

int ProjectConfigManager::getLength() const {
    return this->configList->size();
}

void ProjectConfigManager::set(const int index, const QVariant &value) {
    ProjectConfigType *configItem = getItem(index);
    if (configItem) {
        configItem->set_itemValue(value);
    } else {
        Logger::error(QString("无法设置索引为 %1 的配置项：索引越界").arg(index));
    }
}

void ProjectConfigManager::set(PCE configValue, const QVariant &value) {
    int index = static_cast<int>(configValue);
    ProjectConfigType *configItem = getItem(index);
    if (configItem) {
        configItem->set_itemValue(value);
    } else {
        Logger::error(QString("无法设置索引为 %1 的配置项：索引越界").arg(index));
    }
}

void ProjectConfigManager::appendToStringList(int index, const QString &value) {
    QVariant rawList = this->get(index);
    QStringList stringList = rawList.toStringList();
    stringList.append(value);
    this->set(index, QVariant(stringList));
}

void ProjectConfigManager::appendToStringList(PCE configValue, const QString &value) {
    int index = static_cast<int>(configValue);
    this->appendToStringList(index, value);
}

void ProjectConfigManager::removeFromStringList(int index, const QString &value) {
    QStringList stringList = this->get(index).toStringList();
    stringList.removeOne(value);
    this->set(index, QVariant(stringList));
}

void ProjectConfigManager::removeFromStringList(int index, int valueIndex) {
    QStringList stringList = this->get(index).toStringList();
    stringList.removeAt(valueIndex);
    this->set(index, QVariant(stringList));
}

void ProjectConfigManager::removeFromStringList(PCE configValue, const QString &value) {
    this->removeFromStringList(static_cast<int>(configValue), value);
}

void ProjectConfigManager::removeFromStringList(PCE configValue, int valueIndex) {
    this->removeFromStringList(static_cast<int>(configValue), valueIndex);
}

void ProjectConfigManager::setList(QList<ProjectConfigType *> *list) {
    this->configList = list;
}

void ProjectConfigManager::setDefaultValue() const {
    for (ProjectConfigType *config: *this->configList) {
        QVariant value = config->get_itemValue();
        switch (config->get_itemValue().type()) {
            case QVariant::String:
                config->set_itemValue("");
                break;
            case QVariant::StringList:
                config->set_itemValue(QStringList());
                break;
            case QVariant::Bool:
                if (config->get_itemName() == "onefile" || config->get_itemName() == "removeOutput") {
                    config->set_itemValue(false);
                } else {
                    config->set_itemValue(true);
                }
                break;
            default:
                int ltoModeId = qMetaTypeId<LTOMode>();
                if (value.userType() == ltoModeId) {
                    config->set_itemValue(QVariant::fromValue<LTOMode>(LTOMode::Auto));
                }
        }
    }
}
