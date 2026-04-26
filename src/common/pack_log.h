//
// Created by redrch in NuitkaStudio on 2026/4/26.
// Apache 2.0 license
//

#ifndef PACK_LOG_H
#define PACK_LOG_H

#include <memory>
#include <QJsonObject>
#include <QFile>
#include "types/data_structs.h"

class PackLogPage;

class PackLog : public QObject {
    Q_OBJECT

public:
    PackLog(PackLogPage *packLog, bool isLoadLog = false);
    ~PackLog() override;

    void loadLog();
    void saveNote() const;
    void updateLog() const;

    void setNote(int index, const QString &note) const;

private:
    PackLogPage* packLogPage;
    QList<std::shared_ptr<PackLogStruct>>* logList{};
    QJsonObject noteObject{};
    QFile* noteFile;
};



#endif //PACK_LOG_H
