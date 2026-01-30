//
// Created by redrch on 2026/1/7.
//

#include "project_config.h"

ProjectConfig::ProjectConfig(QWidget *parent) {
    this->parent = parent;
}

ProjectConfig::~ProjectConfig() {
}

void ProjectConfig::importProject(const QString &path) {
    QString filePath;
    if (path.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(this->parent, "Nuitka Studio  导入项目文件",
                                                Config::instance().getDefaultDataPath(),
                                                "Nuitka Project File(*.npf);;All files(*)");
        if (filePath.isEmpty()) {
            return;
        }
    } else {
        filePath = path;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Open failed: " << file.errorString();
        QMessageBox::critical(this->parent, "Nuitka Studio Error", "Open failed: " + file.errorString());
        return;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_5_14);
    QList<ProjectConfigType> items;
    in >> items;

    auto itemList = new QList<ProjectConfigType *>();
    for (const ProjectConfigType &item: items) {
        itemList->append(new ProjectConfigType(item));
    }
    ProjectConfigManager::instance().setList(itemList);

    file.close();

    Logger::info("导入NPF文件");
}

void ProjectConfig::exportProject(const QString &path) {
    QString filePath = "";
    if (path == "") {
        filePath = QFileDialog::getSaveFileName(this->parent, "Nuitka Studio  导出项目文件",
                                                Config::instance().getDefaultDataPath(),
                                                "Nuitka Project File(*.npf);;All files(*)");
        if (filePath.isEmpty()) {
            return;
        }
    } else {
        filePath = path;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Open failed: " << file.errorString();
        QMessageBox::warning(this->parent, "Nuitka Studio  Warning", "Open failed: " + file.errorString());
        return;
    }

    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_5_14);
    QList<ProjectConfigType> itemList;
    for (const ProjectConfigType *item: *ProjectConfigManager::instance().getList()) {
        itemList.append(*item);
    }
    out << itemList;

    file.close();
    Logger::info("导出NPF文件");
}
