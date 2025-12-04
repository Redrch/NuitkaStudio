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

#include <QString>
#include <QList>
#include <QListWidgetItem>
#include <QScopedPointer>

#include <QDebug>

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


private:
    Ui::MainWindow *ui;

    QString pythonPath;
    QString mainFilePath;
    QString outputPath;
    QString outputFilename;
    QString iconPath = QString("");
    LTOMode ltoMode = LTOMode::Auto;

    QList<QString> dataList = QList<QString>();

private slots:
    void on_AddDataFileItem_clicked();
    void on_AddDataDirItem_clicked();
    void on_RemoveItem_clicked();
    void startPack();
};


#endif //NUITKASTUDIO_MAINWINDOW_H
