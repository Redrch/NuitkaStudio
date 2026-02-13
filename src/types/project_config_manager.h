//
// Created by redrch on 2026/1/16.
//

#pragma once

#ifndef PROJECTCONFIGTYPEMANAGER_H
#define PROJECTCONFIGTYPEMANAGER_H

#include "project_config_type.h"
#include "../utils/logger.h"
#include "../utils/utils.h"
#include "data_structs.h"

class ProjectConfigManager {
public:
    static ProjectConfigManager &instance() {
        static ProjectConfigManager instance;
        return instance;
    }

    ProjectConfigManager(const ProjectConfigManager &) = delete;
    ProjectConfigManager &operator=(const ProjectConfigManager &) = delete;

    ProjectConfigManager();
    ~ProjectConfigManager();

    void addItem(ProjectConfigType *config) const;
    void setList(QList<ProjectConfigType *> *list);
    int getLength() const;
    QList<ProjectConfigType *> *getList() const;
    int getIndex(const QString &name) const;

    ProjectConfigType *getItem(int index);
    ProjectConfigType *getItem(ConfigValue value);

    QVariant getItemValue(int index);
    QVariant getItemValue(ConfigValue value);
    QString getItemValueToString(int index);
    QString getItemValueToString(ConfigValue value);
    QStringList getItemValueToStringList(int index);
    QStringList getItemValueToStringList(ConfigValue value);
    bool getItemValueToBool(int index);
    bool getItemValueToBool(ConfigValue value);

    void setItem(int index, const QVariant& value);
    void setItem(ConfigValue configValue, const QVariant& value);
    void appendItemToStringList(int index, const QString& value);
    void appendItemToStringList(ConfigValue configValue, const QString& value);
    void removeItemFromStringList(int index, const QString& value);
    void removeItemFromStringList(int index, int valueIndex);
    void removeItemFromStringList(ConfigValue configValue, const QString& value);
    void removeItemFromStringList(ConfigValue configValue, int valueIndex);

private:
    QList<ProjectConfigType *> *configList;
};


#endif //PROJECTCONFIGTYPEMANAGER_H
