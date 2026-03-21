//
// Created by redrch on 2026/1/7.
//

#include "project_config.h"

ProjectConfig::ProjectConfig() = default;

ProjectConfig::~ProjectConfig() = default;

NPFStatusType ProjectConfig::loadProject(const QString &path) {
    if (!QFile::exists(path)) {
        return NPFStatusType::NPFNotFound;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return NPFStatusType::NPFNotOpen;
    }
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject root = doc.object();

    // npf version
    if (root.value("npf_version").toString() != NPF_VERSION) {
        Logger::error("ProjectConfig::loadProject: 此npf文件的格式版本错误，请尝试更换文件");
        return NPFStatusType::NPFVersionError;
    }

    // project
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
    GDM.setString(GDIN::npfFilePath, path);
    config.setString(ConfigItem::NpfPath, path);
    config.writeConfig();
    Logger::info("导入NPF文件");

    // pack log
    if (root.contains("pack_log")) {
        QString packLogRoot = GDM.getString(GDIN::packLogPath) + "/" + QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName();
        if (!QDir(packLogRoot).exists()) {
            if (!QDir().mkdir(packLogRoot)) {
                Logger::warn("无法创建文件夹" + packLogRoot);
            }
        }
        QJsonObject packLog = root.value("pack_log").toObject();
        QJsonObject notes;
        for (auto item = packLog.begin(); item != packLog.end(); ++item) {
            QString fileName = item.key();
            QJsonArray array = item.value().toArray();

            if (array.size() < 2) {
                Logger::warn(QString("pack_log 项 '%1' 格式不完整，已跳过").arg(fileName));
                continue;
            }

            QString content = array[0].toString();
            QString note = array[1].toString();
            notes.insert(fileName, note);

            QFile logFile(packLogRoot + "/" + fileName);
            logFile.open(QIODevice::WriteOnly);
            logFile.write(content.toUtf8());
            logFile.close();
        }

        QJsonDocument noteDoc(notes);
        QFile noteFile(packLogRoot + "/note.json");
        noteFile.open(QIODevice::WriteOnly);
        noteFile.write(noteDoc.toJson());
        noteFile.close();
    }

    GDM.setString(GDIN::npfFilePath, path);
    return NPFStatusType::NPFRight;
}

NPFStatusType ProjectConfig::saveProject(const QString &path, bool savePackLog) {
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

    if (savePackLog) {
        QString packLogRoot = GDM.getString(GDIN::packLogPath) + "/" + QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName();
        QJsonObject packLog;
        QStringList packLogFileList = QDir(packLogRoot).entryList(QDir::Files);

        // read note
        bool hasNote = packLogFileList.contains("note.json");
        if (!hasNote) {
            Logger::error("无法找到note.json文件，保存中断");
            return NPFStatusType::NotFoundNote;
        }
        packLogFileList.removeOne("note.json");
        QFile noteFile(packLogRoot + "/note.json");
        if (!noteFile.open(QIODevice::ReadOnly)) {
            Logger::error("无法打开note.json文件，保存中断");
            return NPFStatusType::NotFoundNote;
        }
        QByteArray noteJsonText = noteFile.readAll();
        noteFile.close();
        QJsonDocument noteDoc = QJsonDocument::fromJson(noteJsonText);
        QJsonObject noteObject = noteDoc.object();

        // read log
        for (const QString& packLogFile: packLogFileList) {
            QFile file(packLogRoot + "/" + packLogFile);
            if (!file.open(QIODevice::ReadOnly)) {
                Logger::error("无法打开打包日志文件" + packLogFile);
            }
            QString packLogText = QString::fromUtf8(file.readAll());
            file.close();
            QString note = noteObject.value(packLogFile).toString();
            QJsonArray packLogArray;
            packLogArray << packLogText;
            packLogArray << note;
            packLog.insert(packLogFile, packLogArray);
        }

        root.insert("pack_log", packLog);
    }

    // write
    QJsonDocument doc(root);
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return NPFStatusType::NPFNotOpen;
    }
    file.write(doc.toJson());
    file.close();
    Logger::info("导出NPF文件");
    return NPFStatusType::NPFRight;
}
