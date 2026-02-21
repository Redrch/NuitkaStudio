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
    ProjectConfigType *getItem(PCE value);

    QVariant getItemValue(int index);
    QVariant getItemValue(PCE value);
    QString getItemValueToString(int index);
    QString getItemValueToString(PCE value);
    QStringList getItemValueToStringList(int index);
    QStringList getItemValueToStringList(PCE value);
    bool getItemValueToBool(int index);
    bool getItemValueToBool(PCE value);

    void setItem(int index, const QVariant& value);
    void setItem(PCE configValue, const QVariant& value);
    void appendItemToStringList(int index, const QString& value);
    void appendItemToStringList(PCE configValue, const QString& value);
    void removeItemFromStringList(int index, const QString& value);
    void removeItemFromStringList(int index, int valueIndex);
    void removeItemFromStringList(PCE configValue, const QString& value);
    void removeItemFromStringList(PCE configValue, int valueIndex);

    void setDefaultValue() const;

private:
    QList<ProjectConfigType *> *configList;
};


#endif //PROJECTCONFIGTYPEMANAGER_H
