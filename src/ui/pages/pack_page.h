//
// Created by redrch on 2026/4/4.
//

#ifndef PACK_PAGE_H
#define PACK_PAGE_H

#include <QWidget>
#include <QStringListModel>
#include <ElaPlainTextEdit.h>
#include <ElaPushButton.h>

class PackPage : public QWidget {
    Q_OBJECT

public:
    PackPage(QWidget *parent = nullptr);
    void addConsoleContent(const QString &content) const;
    void packStart();
    void packEnd();

private:
    QStringListModel* assetListModel;
    bool isPacking;

    ElaPlainTextEdit* consoleEdit;
    ElaPushButton* startPackButton;
    ElaPushButton* stopPackButton;

signals:
    Q_SIGNAL void startPack();
    Q_SIGNAL void stopPack();
};


#endif //PACK_PAGE_H
