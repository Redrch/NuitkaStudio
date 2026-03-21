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
    NPFNotOpen,
    NotFoundNote,
    NPFRight
};

class ProjectConfig {
public:
    ProjectConfig();
    ~ProjectConfig();

    static NPFStatusType loadProject(const QString& path) ;

    static NPFStatusType saveProject(const QString& path, bool savePackLog);
};



#endif //PROJECTCONFIG_H
