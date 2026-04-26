//
// Created by redrch in NuitkaStudio on 2026/4/26.
// Apache 2.0 license
//

#ifndef PACK_LOG_PAGE_H
#define PACK_LOG_PAGE_H

#include <QWidget>
#include <QStringListModel>
#include <QList>

#include "types/data_structs.h"

class PackLog;

class PackLogPage : public QWidget {
    Q_OBJECT

public:
    explicit PackLogPage(QWidget *parent = nullptr);
    ~PackLogPage() override;

    void setPackLogObj(PackLog *log);
    void addLog(std::shared_ptr<PackLogStruct> log);
    void setLog(int index, std::shared_ptr<PackLogStruct> log);
    void removeLog(int index);
    void removeAllLogs();
    int logIndex(std::shared_ptr<PackLogStruct> log) const;

private:
    QStringListModel *logModel;
    QList<std::shared_ptr<PackLogStruct>> packLogs;
    int currentRow{};
    PackLog *packLogObj;
};



#endif //PACK_LOG_PAGE_H
