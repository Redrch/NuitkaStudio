//
// Created by redrch on 2026/2/18.
//

#ifndef PACK_LOG_WINDOW_H
#define PACK_LOG_WINDOW_H

#include <QWidget>
#include <QMap>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
namespace Ui { class PackLogWindow; }
QT_END_NAMESPACE

class PackLogWindow : public QWidget {
Q_OBJECT

public:
    explicit PackLogWindow(QWidget *parent = nullptr);
    ~PackLogWindow() override;

    void setLog(const QMap<QString, QString> &log);

private:
    Ui::PackLogWindow *ui;
    QMap<QString, QString> log;
};


#endif //PACK_LOG_WINDOW_H
