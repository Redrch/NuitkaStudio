//
// Created by redrch on 2026/1/7.
//

#ifndef PROJECTCONFIG_H
#define PROJECTCONFIG_H

#include <QString>
#include <QFileDialog>
#include <QMessageBox>

#include "data_structs.h"
#include "config.h"
#include "logger.h"
#include "utils.h"

class ProjectConfig {
public:
    ProjectConfig(ProjectConfigData* data, QWidget* parent);
    ~ProjectConfig();

    void importProject(const QString& path = "");
    void exportProject(const QString& path = "");
private:
    ProjectConfigData* data;
    QWidget *parent;
};



#endif //PROJECTCONFIG_H
