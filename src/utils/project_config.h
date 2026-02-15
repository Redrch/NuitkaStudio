//
// Created by redrch on 2026/1/7.
//

#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

#include "../types/data_structs.h"
#include "../types/project_config_manager.h"
#include "../types/simname.h"
#include "compress.h"
#include "config.h"
#include "logger.h"
#include "utils.h"



class ProjectConfig {
public:
    explicit ProjectConfig(QWidget* parent);
    ~ProjectConfig();

    QString importProject(const QString& path = "") const;
    QString exportProject(const QString& path = "") const;
private:
    QWidget *parent;
    Compress* compress;
};



#endif //PROJECTCONFIG_H
