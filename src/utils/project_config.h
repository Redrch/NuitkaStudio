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

enum class NPFStatusType {
    NPFDamage,
    NPFVersionError,
    NPFNotFound,
    NPFRight
};

class ProjectConfig {
public:
    ProjectConfig();
    ~ProjectConfig();

    NPFStatusType loadProject(const QString& path) const;
    NPFStatusType saveProject(const QString& path) const;
private:
    Compress* compress;
};



#endif //PROJECTCONFIG_H
