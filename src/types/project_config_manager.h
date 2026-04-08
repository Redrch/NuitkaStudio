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

    ProjectConfigType *getItem(int index) const;
    ProjectConfigType *getItem(PCE value);

    QVariant get(int index);
    QVariant get(PCE value);
    QString getString(int index);
    QString getString(PCE value);
    QStringList getStringList(int index);
    QStringList getStringList(PCE value);
    bool getBool(int index);
    bool getBool(PCE value);

    void set(int index, const QVariant& value);
    void set(PCE configValue, const QVariant& value);
    void appendToStringList(int index, const QString& value);
    void appendToStringList(PCE configValue, const QString& value);
    void removeFromStringList(int index, const QString& value);
    void removeFromStringList(int index, int valueIndex);
    void removeFromStringList(PCE configValue, const QString& value);
    void removeFromStringList(PCE configValue, int valueIndex);

    void setDefaultValue() const;

private:
    QList<ProjectConfigType *> *configList;
};


#endif //PROJECTCONFIGTYPEMANAGER_H
