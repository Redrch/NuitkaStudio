//
// Created by redrch on 2026/1/7.
//

#include "project_config.h"

ProjectConfig::ProjectConfig() {
    this->compress = new Compress;
}

ProjectConfig::~ProjectConfig() {
    delete this->compress;
}

NPFStatusType ProjectConfig::loadProject(const QString &path) const {
    if (!QFile::exists(path)) {
        return NPFStatusType::NPFNotFound;
    }
    this->compress->setZipPath(path);
    QByteArray data = this->compress->readZip("data.json");

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        Logger::error(QString("ProjectConfig::loadProject: npf文件%1已损坏，请尝试更换文件").arg(path));
        return NPFStatusType::NPFDamage;
    }
    QJsonObject root = doc.object();
    if (root.value("npf_version") != NPF_VERSION) {
        Logger::error("ProjectConfig::loadProject: 此npf文件的格式版本错误，请尝试更换文件");
        return NPFStatusType::NPFVersionError;
    }
    QJsonObject project = root.value("project").toObject();
    for (auto item = project.begin(); item != project.end(); ++item) {
        QString key = item.key();
        QVariant value{item.value()};

        if (value.userType() >= QMetaType::User && value.canConvert<int>()) {
            value = value.toInt();
        }
        const int index = PCM.getIndex(key);
        if (index == -1) {
            Logger::error("ProjectConfig::loadProject: 此npf文件已损坏，请尝试更换文件");
            return NPFStatusType::NPFDamage;
        }
        PCM.setItem(index, value);
    }
    GDM.setString(GDIN::NPF_FILE_PATH, path);
    config.setConfigFromString(SettingsEnum::NpfPath, path);
    config.writeConfig();
    Logger::info("导入NPF文件");
    return NPFStatusType::NPFRight;
}

NPFStatusType ProjectConfig::saveProject(const QString &path) const {
    this->compress->setZipPath(path);

    if (QFile::exists(path)) {
        QFile::remove(path);
    }

    QString jsonPath = "data.json";

    // json string
    QJsonObject root;
    root.insert("npf_version", NPF_VERSION);

    QJsonObject project;
    for (const ProjectConfigType *item: *PCM.getList()) {
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
    GDM.setString(GDIN::NPF_FILE_PATH, path);
    config.setConfigFromString(SettingsEnum::NpfPath, path);
    config.writeConfig();
    Logger::info("导出NPF文件");
    return NPFStatusType::NPFRight;
}
