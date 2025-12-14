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
#include <QMessageBox>

#include <QString>
#include <QList>
#include <QScopedPointer>

#include <QFile>
#include <QProcess>
#include <QDateTime>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>

#include <QDebug>

#include "export_datalist_window.h"
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
    bool onefile = false;
    bool removeOutput = false;

    LTOMode ltoMode = LTOMode::Auto;

    QList<QString> dataList = QList<QString>();

    QString processErrorToString(QProcess::ProcessError err);
    void updateExportTable();
    void updatePackUI();
    void updateUI();

private slots:
    void on_AddDataFileItem_clicked();
    void on_AddDataDirItem_clicked();
    void on_RemoveItem_clicked();
    void on_ProjectTable_cellDoubleClicked(int row, int column);

    void startPack();
    void importProject();
    void exportProject();
};


#endif //NUITKASTUDIO_MAINWINDOW_H
