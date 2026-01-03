//
// Created by redrch on 2025/11/30.
//

#ifndef NUITKASTUDIO_MAINWINDOW_H
#define NUITKASTUDIO_MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QConstOverload>
#include <QMessageBox>

#include <QString>
#include <QList>

#include <QProcess>
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

namespace Ui {
    class MainWindow;
}

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
    QString iconPath;

    bool standalone = true;
    bool onefile = false;
    bool removeOutput = false;
    LTOMode ltoMode = LTOMode::Auto;
    QList<QString> dataList = QList<QString>();

    QDateTime startPackTime;

    QTimer *packTimer;
    QProcess *packProcess;

    QCheckBox *standaloneCheckbox;
    QCheckBox *onefileCheckbox;
    QCheckBox *removeOutputCheckbox;
    QComboBox *ltoModeCombobox;
    QLabel *timerLabel;

    // functions
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

    // Init functions
    void initExportPage();
    void initStatusBar();

    // Util functions
    static QString boolToString(bool v);
    static QString processErrorToString(QProcess::ProcessError err);
    static QString formatMilliseconds(qint64 totalMs);

private slots:
    void onAddDataFileItemClicked();
    void onAddDataDirItemClicked();
    void onRemoveItemClicked();

    void onProjectTableCellDoubleClicked(int row, int column);

    void onFileMenuTriggered(QAction *action);
    void onHelpMenuTriggered(QAction *action);

    void startPack();
    void stopPack();

    void importProject();
    void exportProject();
};


#endif //NUITKASTUDIO_MAINWINDOW_H
