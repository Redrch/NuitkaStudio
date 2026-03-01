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
#include <QStringListModel>
#include <QPointer>

#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QCloseEvent>
#include <QCursor>

#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QProcessEnvironment>

#include <QDebug>

#include <ElaWindow.h>
#include <ElaTabBar.h>
#include <ElaMenuBar.h>
#include <ElaTheme.h>
#include <ElaIconButton.h>

#include "about_window.h"
#include "float_button.h"

#include "../types/data_structs.h"
#include "../types/project_config_manager.h"
#include "../types/simname.h"

#include "../utils/utils.h"
#include "../utils/config.h"
#include "../utils/logger.h"
#include "../utils/project_config.h"
#include "../utils/compress.h"
#include "../types/color.h"

QT_BEGIN_NAMESPACE

namespace Ui {
    class MainWindow;
}
namespace PackPageUi {
    class PackPage;
}

enum class TextPos {
    TopLabel,
    Statusbar,
    SystemMessage
};

QT_END_NAMESPACE

class MainWindow : public ElaWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    // variants
    Ui::MainWindow *ui;

    ProjectConfig *projectConfig;

    QDateTime startPackTime;
    QTimer *packTimer;
    QTimer *mainTimer;
    QProcess *packProcess;
    // bar
    ElaTabBar *topTabBar;
    ElaMenuBar *menuBar;
    // controls
    QCheckBox *standaloneCheckbox;
    QCheckBox *onefileCheckbox;
    QCheckBox *removeOutputCheckbox;
    QComboBox *ltoModeCombobox;
    QLabel *messageLabel;
    QLabel *topTextLabel;
    FloatButton* floatButton;
    // tray menu
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *startPackAction;
    QAction *stopPackAction;
    QAction *showAction;
    QAction *quitAction;
    // pack log
    QList<PackLog> *packLog;
    // models
    QStringListModel *packLogModel;
    QStringListModel *dataListModel;

    // functions
    // Update UI functions
    void updatePackUI() const;
    void updateSettingsUI() const;
    void updatePackLogUI();
    void updateUI();

    // Connect signals and slots functions
    void connectStackedWidget();
    void connectMenubar();
    void connectPackPage();
    void connectSettingsPage();
    void connectTrayMenu();
    void connectOther();
    void connectPackLog();

    // Init functions
    void initUI();
    void initMenuBar();

    // ui util functions
    /**
     * MainWindow::showText
     * @param text Showed text.
     * @param showTime Showed time, -1 indicates a permanent show. However, it does not apply in system notifications and defaults to 5000.
     * @param color Showed text color, default is black.
     * @param position Showed text position, default is on the top label.
     * @param title This parameter is only used when position is TextPos::SystemMessage,
     *              serving as the title for system notifications, and defaults to Nuitka Studio.
     */
    void showText(const QString &text, int showTime = -1, const QColor &color = Qt::black,
                  TextPos position = TextPos::TopLabel, const QString &title = "Nuitka Studio") const;
    void clearText(TextPos position = TextPos::TopLabel) const;
    void enabledInput() const;
    void noEnableInput() const;

    // util functions
    void readPackLog();
    bool npfStatusTypeHandler(NPFStatusType status, const QString& path, bool isTip = true);

private slots:
    void onAddDataFileItemClicked();
    void onAddDataDirItemClicked();
    void onRemoveItemClicked();

    void onFileMenuTriggered(QAction *action);
    void onHelpMenuTriggered(QAction *action);

    void startPack();
    void stopPack();

    void importProject();
    void exportProject();

    // Gen path functions
    void genData(bool isUpdateUI = true);
    void genPythonPath();
    void genMainfilePath();
    void genOutputPath();
    void genOutputName();
    // Gen file info functions
    void genFileInfo();

protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
};


#endif //NUITKASTUDIO_MAINWINDOW_H
