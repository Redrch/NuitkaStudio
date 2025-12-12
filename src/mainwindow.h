//
// Created by redrch on 2025/11/30.
//

#ifndef NUITKASTUDIO_MAINWINDOW_H
#define NUITKASTUDIO_MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QPushButton>
#include <QFileDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QConstOverload>
#include <QListWidgetItem>
#include <QTableWidgetItem>

#include <QString>
#include <QList>
#include <QScopedPointer>

#include <QFile>
#include <QProcess>
#include <QDateTime>
#include <QTimer>

#include <QDebug>

#include "config.h"

enum class LTOMode {
    No,
    Yes,
    Auto
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    static QString boolToString(bool v);

private:
    Ui::MainWindow *ui;

    QString pythonPath;
    QString mainFilePath;
    QString outputPath;
    QString outputFilename;
    QString iconPath;

    bool standalone;
    bool onefile;
    bool removeOutput;

    LTOMode ltoMode = LTOMode::Auto;

    Config* config;

    QList<QString> dataList = QList<QString>();

    QString processErrorToString(QProcess::ProcessError err);
    void updateExportTable();

private slots:
    void on_AddDataFileItem_clicked();
    void on_AddDataDirItem_clicked();
    void on_RemoveItem_clicked();
    void startPack();
};


#endif //NUITKASTUDIO_MAINWINDOW_H
