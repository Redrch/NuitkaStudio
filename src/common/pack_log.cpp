//
// Created by redrch in NuitkaStudio on 2026/4/26.
// Apache 2.0 license
//

#include "pack_log.h"
#include "ui/pages/pack_log_page.h"
#include "common/simname.h"
#include <QJsonDocument>

PackLog::PackLog(PackLogPage *packLog, bool isLoadLog) {
    this->noteFile = new QFile(
        GDIN::packLogPath + "/" + QFileInfo(GDM.getString(GDIN::npfFilePath)).fileName() + "/note.json");
    this->packLogPage = packLog;
    this->logList = new QList<std::shared_ptr<PackLogStruct>>();

    if (isLoadLog) this->loadLog();
}

PackLog::~PackLog() {
    delete this->logList;
    delete this->noteFile;
}

void PackLog::loadLog() {
    // gen path
    this->logList->clear();
    if (GDM.getString(GDIN::npfFilePath).isEmpty()) {
        Logger::warn("NPF文件路径为空，无法调用PackLog::loadLog函数");
        return;
    }
    QString packLogRoot = GDM.getString(GDIN::packLogPath) + "/" + QFileInfo(GDM.getString(GDIN::npfFilePath)).
                          fileName();

    if (!QDir(packLogRoot).exists()) {
        Logger::warn("PackLog::loadLog(): 打包日志目录不存在, " + packLogRoot);
        return;
    }
    QStringList packLogFileList = QDir(packLogRoot).entryList(QDir::Files);
    if (packLogFileList.contains("note.json")) {
        packLogFileList.removeOne("note.json");
    }

    // load note
    if (this->noteFile->open(QIODevice::ReadOnly)) {
        this->noteObject = QJsonDocument::fromJson(this->noteFile->readAll()).object();
        this->noteFile->close();
    }

    // load file
    for (const QString &packLogFile: packLogFileList) {
        QFile file(packLogRoot + "/" + packLogFile);
        if (!file.open(QIODevice::ReadOnly)) {
            return;
        }

        QString packLog = QString::fromUtf8(file.readAll());
        QString note = this->noteObject.value(packLogFile).toString();

        this->logList->append(std::make_shared<PackLogStruct>(packLogFile, packLog, note));
    }
}

void PackLog::saveNote() const {
    this->noteFile->open(QIODevice::WriteOnly);
    this->noteFile->write(QJsonDocument(this->noteObject).toJson());
    this->noteFile->close();
}

void PackLog::updateLog() const {
    if (!this->logList) {
        return;
    }
    this->packLogPage->removeAllLogs();
    for (std::shared_ptr log: *this->logList) {
        packLogPage->addLog(log);
    }
}

void PackLog::setNote(int index, const QString &note) const {
    std::shared_ptr log = this->logList->at(index);
    log->logNote = note;
}
