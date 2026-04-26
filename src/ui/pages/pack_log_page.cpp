//
// Created by redrch in NuitkaStudio on 2026/4/26.
// Apache 2.0 license
//

#include "pack_log_page.h"
#include <QHBoxLayout>
#include <ElaText.h>
#include <ElaLineEdit.h>
#include <ElaListView.h>
#include <ElaPlainTextEdit.h>
#include "common/pack_log.h"

PackLogPage::PackLogPage(QWidget *parent) : QWidget(parent) {
    this->setMinimumSize(950, 570);

    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // title
    QWidget *titleWidget = new QWidget(this);
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    ElaText *titleText = new ElaText(tr("打包日志"), 16, titleWidget);
    titleLayout->addWidget(titleText);
    titleLayout->addStretch();
    mainLayout->addWidget(titleWidget);

    // note
    QWidget *noteWidget = new QWidget(this);
    QHBoxLayout *noteLayout = new QHBoxLayout(noteWidget);
    ElaText *noteText = new ElaText(tr("备注"), 12, noteWidget);
    ElaLineEdit *noteEdit = new ElaLineEdit(noteText);
    noteLayout->addWidget(noteText);
    noteLayout->addWidget(noteEdit);
    mainLayout->addWidget(noteWidget);

    // pack log
    QWidget *packLogWidget = new QWidget(this);
    QHBoxLayout *packLogLayout = new QHBoxLayout(packLogWidget);
    ElaListView *packLogListView = new ElaListView(packLogWidget);
    this->logModel = new QStringListModel();
    packLogListView->setModel(this->logModel);
    ElaPlainTextEdit* logText = new ElaPlainTextEdit(packLogWidget);
    logText->setReadOnly(true);
    packLogLayout->addWidget(packLogListView);
    packLogLayout->addWidget(logText);
    packLogLayout->setStretch(0, 1);
    packLogLayout->setStretch(1, 5);
    mainLayout->addWidget(packLogWidget);

    connect(packLogListView, &ElaListView::clicked, this, [=](const QModelIndex &index) {
        int row = index.row();
        this->currentRow = row;
        if (this->currentRow < 0 || this->currentRow >= this->packLogs.size()) return;
        PackLogStruct* logData = this->packLogs.at(row).get();
        logText->setPlainText(logData->logContent);
        noteEdit->setText(logData->logNote);
    });
    connect(noteEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
        if (this->currentRow < 0 || this->currentRow >= this->packLogs.size()) return;
        PackLogStruct* logData = this->packLogs.at(this->currentRow).get();
        logData->logNote = text;
        if (packLogObj) {
            this->packLogObj->setNote(this->currentRow, text);
        }
    });
}

PackLogPage::~PackLogPage() {
    this->removeAllLogs();
    delete this->logModel;
}

void PackLogPage::setPackLogObj(PackLog *log) {
    this->packLogObj = log;
}

void PackLogPage::addLog(std::shared_ptr<PackLogStruct> log) {
    this->packLogs.append(log);
    QStringList logList = this->logModel->stringList();
    logList.append(log->logFileName);
    this->logModel->setStringList(logList);
}

void PackLogPage::setLog(int index, std::shared_ptr<PackLogStruct> log) {
    if (index < 0 || index >= this->packLogs.size()) return;
    this->packLogs.replace(index, log);
}

void PackLogPage::removeLog(int index) {
    this->packLogs.removeAt(index);
}

void PackLogPage::removeAllLogs() {
    this->packLogs.clear();
}

int PackLogPage::logIndex(std::shared_ptr<PackLogStruct> log) const {
    for (int i = 0; i < this->packLogs.size(); ++i) {
        if (this->packLogs.at(i) == log) return i;
    }
    return -1;
}


