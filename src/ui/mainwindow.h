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
#include <QProcessEnvironment>

#include <QDebug>

#include "export_datalist_window.h"
#include "about_window.h"
#include "../utils/config.h"
#include "../utils/logger.h"
#include "new_project_window.h"
#include "../types/data_structs.h"
#include "../types/project_config_manager.h"
#include "../utils/utils.h"
#include "../utils/project_config.h"
#include "../utils/compress.h"
#include "../types/pcm.h"

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

    ProjectConfig *projectConfig;

    QDateTime startPackTime;
    QTimer *packTimer;
    QProcess *packProcess;

    QCheckBox *standaloneCheckbox;
    QCheckBox *onefileCheckbox;
    QCheckBox *removeOutputCheckbox;
    QComboBox *ltoModeCombobox;
    QLabel *messageLabel;

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
    void newProject();

    // Gen path functions
    void genData(bool isUpdateUI = true);
    void genPythonPath();
    void genMainfilePath();
    void genOutputPath();
    void genOutputName();
    // Gen file info functions
    void genFileInfo();
};


#endif //NUITKASTUDIO_MAINWINDOW_H
