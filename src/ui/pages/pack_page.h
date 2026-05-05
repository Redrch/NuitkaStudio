//
// Created by redrch on 2026/4/4.
//

#ifndef PACK_PAGE_H
#define PACK_PAGE_H

#include <QStringListModel>
#include <ElaScrollPageArea.h>
#include <ElaScrollArea.h>
#include <ElaPlainTextEdit.h>
#include <ElaPushButton.h>
#include "types/data_structs.h"

class PackPage : public QWidget {
    Q_OBJECT

public:
    explicit PackPage(QWidget *parent = nullptr);
    void addConsoleContent(const QString &content) const;
    void packStart();
    void packEnd();
    void scrollTo(const PageCard &card) const;

private:
    QStringListModel* assetListModel;
    bool isPacking;

    ElaPlainTextEdit* consoleEdit;
    ElaPushButton* startPackButton;
    ElaPushButton* stopPackButton;
    ElaScrollArea* scrollArea;

    ElaScrollPageArea *baseCard;
    ElaScrollPageArea *packCard;
    ElaScrollPageArea *assetCard;
    ElaScrollPageArea *fileInfoCard;
    ElaScrollPageArea *consoleCard;

signals:
    Q_SIGNAL void startPack();
    Q_SIGNAL void stopPack();
};


#endif //PACK_PAGE_H
