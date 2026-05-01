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
#include <QTranslator>

#include <QDateTime>
#include <QElapsedTimer>
#include <QTimer>
#include <QProcessEnvironment>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDialog>

#include <QDebug>

#include <ElaWindow.h>
#include <ElaTabBar.h>
#include <ElaMenuBar.h>
#include <ElaTheme.h>
#include <ElaIconButton.h>
#include <QStackedWidget>

#include "about_window.h"
#include "float_button.h"

#include "../types/data_structs.h"
#include "../common/project_config_manager.h"
#include "../common/simname.h"

#include "../utils/utils.h"
#include "../common/config.h"
#include "../utils/logger.h"
#include "../utils/project_config.h"
#include "../utils/compress.h"
#include "../types/color.h"
#include "common/pack_log.h"

#include "pages/pack_page.h"
#include "pages/settings_page.h"
#include "pages/pack_log_page.h"

enum class TextPos {
    TopLabel,
    Statusbar,
    SystemMessage
};

class MainWindow : public ElaWindow {
    Q_OBJECT

    const QMap<QString, PageCard> navigationTitleEnumMap = {
        {tr("基础配置"), PageCard::PackPageBaseCard},
        {tr("打包配置"), PageCard::PackPagePackCard},
        {tr("资源配置"), PageCard::PackPageAssetCard},
        {tr("文件信息配置"), PageCard::PackPageFileInfoCard},
        {tr("控制台"), PageCard::PackPageConsoleCard},
        {tr("通用设置"), PageCard::SettingsPageGeneralCard},
        {tr("默认路径设置"), PageCard::SettingsPageDefaultPathCard}
    };

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

private:
    // variants
    QDateTime startPackTime;
    QTimer *packTimer;
    QTimer *mainTimer;
    QProcess *packProcess;
    QFile *noteFile;
    QJsonObject noteObject;

    // controls
    QCheckBox *standaloneCheckbox;
    QCheckBox *onefileCheckbox;
    QCheckBox *removeOutputCheckbox;
    QComboBox *ltoModeCombobox;
    QLabel *messageLabel;
    QLabel *topTextLabel;
    FloatButton* floatButton;
    // status bar
    ElaText *statusLabel;
    ElaText *statusFileNameLabel;

    // actions
    QAction *packAction;
    QAction *settingsAction;
    QAction *packLogAction;
    QAction *floatButtonAction;

    // tray menu
    QSystemTrayIcon *trayIcon;
    QMenu *trayMenu;
    QAction *startPackAction;
    QAction *stopPackAction;
    QAction *showAction;
    QAction *quitAction;

    // pack log
    QList<PackLogStruct*> *packLog;
    PackLog* log;

    // models
    QStringListModel *packLogModel;
    QStringListModel *dataListModel;

    // pages
    PackPage *packPage;
    SettingsPage *settingsPage;
    PackLogPage *packLogPage;

    int currentPageIndex;
    int currentPackLogIndex;

    // functions
    // Connect signals and slots function
    void connectOther();

    // Init function
    void initUI();

    // Gen path functions
    void genData(bool isUpdateUI = true);
    static void genPythonPath();
    static void genMainfilePath();
    static void genOutputPath();
    static void genOutputName();
    // Gen file info functions
    static void genFileInfo();

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
    void disableUi() const;
    void enableUi() const;

    // util functions
    bool npfStatusTypeHandler(NPFStatusType status, const QString& path, bool isTip = true);

private slots:
    void startPack();
    void stopPack();

    void importProject();
    void exportProject();
protected:
    void closeEvent(QCloseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
};


#endif //NUITKASTUDIO_MAINWINDOW_H
