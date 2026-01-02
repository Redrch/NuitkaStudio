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
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>

#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>

#include <QDebug>

#include "export_datalist_window.h"
#include "about_window.h"
#include "config.h"
#include "logger.h"

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

    bool standalone = true;
    bool onefile = false;
    bool removeOutput = false;
    LTOMode ltoMode = LTOMode::Auto;

    QList<QString> dataList = QList<QString>();

    QCheckBox* standaloneCheckbox;
    QCheckBox* onefileCheckbox;
    QCheckBox* removeOutputCheckbox;
    QComboBox* ltoModeCombobox;

    QString processErrorToString(QProcess::ProcessError err);
    // Update UI functions
    void updateExportTable();
    void updatePackUI();
    void updateUI();
    // Connect signals and slots functions
    void connectStackedWidget();
    void connectMenubar();
    void connectPackPage();
    void connectSettingsPage();
    void connectExportPage();

private slots:
    void onAddDataFileItemClicked();
    void onAddDataDirItemClicked();
    void onRemoveItemClicked();
    void onProjectTableCellDoubleClicked(int row, int column);
    void onFileMenuTriggered(QAction* action);
    void onHelpMenuTriggered(QAction* action);

    void startPack();
    void importProject();
    void exportProject();
};


#endif //NUITKASTUDIO_MAINWINDOW_H
