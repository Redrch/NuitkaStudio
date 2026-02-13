//
// Created by redrch on 2026/1/7.
//

#include "project_config.h"

ProjectConfig::ProjectConfig(QWidget *parent) {
    this->parent = parent;
    this->compress = new Compress;
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

    this->compress->setZipPath(filePath);
    QByteArray data = this->compress->readZip("data.json");

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        Logger::error(QString("ProjectConfig::importProject: npf文件%1已损坏，请尝试更换文件").arg(filePath));
        QMessageBox::critical(this->parent, "Nuitka Studio Error", QString("npf文件%1已损坏，请尝试更换文件").arg(filePath));
        return;
    }
    QJsonObject root = doc.object();
    if (root.value("npf_version") != NPF_VERSION) {
        Logger::error("ProjectConfig::importProject: 此npf文件的格式版本错误，请尝试更换文件");
        QMessageBox::critical(this->parent, "Nuitka Studio Error", "此npf文件的格式版本错误，请尝试更换文件");
        return;
    }
    QJsonObject project = root.value("project").toObject();
    for (auto item = project.begin(); item != project.end(); ++item) {
        QString key = item.key();
        QVariant value{item.value()};

        if (value.userType() >= QMetaType::User && value.canConvert<int>()) {
            value = value.toInt();
        }
        const int index = ProjectConfigManager::instance().getIndex(key);
        if (index == -1) {
            Logger::error("ProjectConfig::importProject: 此npf文件已损坏，请尝试更换文件");
            QMessageBox::critical(this->parent, "Nuitka Studio Error", "此npf文件已损坏，请尝试更换文件");
            return;
        }
        ProjectConfigManager::instance().setItem(index, value);
    }

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
    this->compress->setZipPath(filePath);

    if (QFile::exists(filePath)) {
        QFile::remove(filePath);
    }
    Compress::createEmptyZip(filePath);
    QString jsonPath = "data.json";

    // json string
    QJsonObject root;
    root.insert("npf_version", NPF_VERSION);

    QJsonObject project;
    for (const ProjectConfigType *item: *ProjectConfigManager::instance().getList()) {
        QVariant itemValue = item->get_itemValue();
        // another type
        if (itemValue.userType() >= QMetaType::User && itemValue.canConvert<int>()) {
            itemValue = itemValue.toInt();
        }
        project.insert(item->get_itemName(), QJsonValue::fromVariant(itemValue));
    }
    root.insert("project", project);

    QJsonDocument doc(root);
    QString docString = doc.toJson();

    // write
    this->compress->writeZip(jsonPath, docString.toUtf8());

    Logger::info("导出NPF文件");
}
