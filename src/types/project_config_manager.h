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
#include <QObject>
#include <QTimer>

class ProjectConfigManager : public QObject {
    Q_OBJECT
public:
    static ProjectConfigManager &instance() {
        static ProjectConfigManager instance;
        return instance;
    }

    ProjectConfigManager(const ProjectConfigManager &) = delete;
    ProjectConfigManager &operator=(const ProjectConfigManager &) = delete;

    int getUpdateInterval() const;
    void setUpdateInterval(const int value);

    void addItem(ProjectConfigType *config) const;
    void setList(QList<ProjectConfigType *> *list);
    int getLength() const;
    QList<ProjectConfigType *> *getList() const;
    int getIndex(const QString &name) const;

    ProjectConfigType *getItem(int index) const;
    ProjectConfigType *getItem(PCE value) const;

    QVariant get(int index) const;
    QVariant get(PCE value) const;
    QString getString(int index) const;
    QString getString(PCE value) const;
    QStringList getStringList(int index) const;
    QStringList getStringList(PCE value) const;
    bool getBool(int index) const;
    bool getBool(PCE value) const;

    void set(int index, const QVariant& value);
    void set(PCE configValue, const QVariant& value);
    void appendToStringList(int index, const QString& value);
    void appendToStringList(PCE configValue, const QString& value);
    void removeFromStringList(int index, const QString& value);
    void removeFromStringList(int index, int valueIndex);
    void removeFromStringList(PCE configValue, const QString& value);
    void removeFromStringList(PCE configValue, int valueIndex);

    void setDefaultValue() const;

public slots:
    void updateGlobalUI();

private:
    ProjectConfigManager();
    ~ProjectConfigManager() override;

    QList<ProjectConfigType *> *configList;

    int updateInterval;
    int updateCounter;
    QTimer *updateTimer;

signals:
    Q_SIGNAL void updateUI();
};


#endif //PROJECTCONFIGTYPEMANAGER_H
