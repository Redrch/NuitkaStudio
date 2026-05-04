//
// Created by redrch on 2026/4/12.
//

#include "settings_page.h"
#include "global/application.h"
#include "global/update_clock.h"
#include "utils/utils.h"

#include <QVBoxLayout>
#include <QButtonGroup>

#include <ElaScrollArea.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>
#include <ElaComboBox.h>
#include <ElaSpinBox.h>
#include <ElaCheckBox.h>
#include <ElaLineEdit.h>
#include <ElaToggleSwitch.h>
#include <ElaRadioButton.h>
#include <ElaApplication.h>
#include <filesystem>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    this->setMinimumSize(600, 400);
    this->setStyleSheet("background: transparent;");
    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // scroll area
    this->scrollArea = new ElaScrollArea(this);
    QWidget *scrollWidget = new QWidget();
    scrollWidget->setObjectName("centralWidget");

    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    scrollLayout->setAlignment(Qt::AlignTop);
    scrollLayout->setContentsMargins(25, 20, 25, 20);
    scrollLayout->setSpacing(15);
    scrollLayout->setSizeConstraint(QLayout::SetMinimumSize);
    scrollWidget->setLayout(scrollLayout);

    ElaWindow *window = dynamic_cast<ElaWindow *>(parent);

    // General Settings
#pragma region GeneralCardSection
    {
        this->generalCard = new QWidget(scrollWidget);
        generalCard->setFixedHeight(QWIDGETSIZE_MAX);
        generalCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *generalLayout = new QVBoxLayout(generalCard);
        generalLayout->setSpacing(18);
        generalLayout->setContentsMargins(0, 0, 0, 0);
        generalLayout->setAlignment(Qt::AlignTop);
        generalLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("通用设置"), 16, generalCard);
        generalLayout->addWidget(title);

        // show exit confirm
        ElaScrollPageArea *showExitConfirmWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *showExitConfirmLayout = new QHBoxLayout(showExitConfirmWidget);
        ElaText *showExitConfirmText = new ElaText(tr("退出确认提示"), 12, showExitConfirmWidget);
        ElaToggleSwitch *showExitConfirmSwitch = new ElaToggleSwitch(generalCard);
        showExitConfirmLayout->addWidget(showExitConfirmText);
        showExitConfirmLayout->addStretch();
        showExitConfirmLayout->addWidget(showExitConfirmSwitch);
        generalLayout->addWidget(showExitConfirmWidget);

        // hide on exit
        ElaScrollPageArea *hideOnExitWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *hideOnExitLayout = new QHBoxLayout(hideOnExitWidget);
        ElaText *hideOnExitText = new ElaText(tr("退出时最小化到系统托盘"), 12, hideOnExitWidget);
        hideOnExitText->setFixedWidth(200);
        ElaToggleSwitch *hideOnExitSwitch = new ElaToggleSwitch(generalCard);
        hideOnExitLayout->addWidget(hideOnExitText);
        hideOnExitLayout->addStretch();
        hideOnExitLayout->addWidget(hideOnExitSwitch);
        hideOnExitWidget->hide();
        generalLayout->addWidget(hideOnExitWidget);

        // splash screen
        ElaScrollPageArea *splashScreenWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *splashScreenLayout = new QHBoxLayout(splashScreenWidget);
        ElaText *splashScreenText = new ElaText(tr("开屏动画"), 12, splashScreenWidget);
        ElaToggleSwitch *splashScreenSwitch = new ElaToggleSwitch(generalCard);
        splashScreenLayout->addWidget(splashScreenText);
        splashScreenLayout->addStretch();
        splashScreenLayout->addWidget(splashScreenSwitch);
        generalLayout->addWidget(splashScreenWidget);

        // max pack log count
        ElaScrollPageArea *maxPackLogCountWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *maxPackLogCountLayout = new QHBoxLayout(maxPackLogCountWidget);
        ElaText *maxPackLogCountText = new ElaText(tr("最大日志数量"), 12, maxPackLogCountWidget);
        ElaSpinBox *maxPackLogCountSpinBox = new ElaSpinBox(maxPackLogCountWidget);
        maxPackLogCountSpinBox->installEventFilter(this);
        maxPackLogCountSpinBox->setButtonMode(ElaSpinBoxType::Compact);
        maxPackLogCountLayout->addWidget(maxPackLogCountText);
        maxPackLogCountLayout->addStretch();
        maxPackLogCountLayout->addWidget(maxPackLogCountSpinBox);
        generalLayout->addWidget(maxPackLogCountWidget);

        // save pack log
        ElaScrollPageArea *savePackLogWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *savePackLogLayout = new QHBoxLayout(savePackLogWidget);
        ElaText *savePackLogText = new ElaText(tr("保存打包日志"), 12, savePackLogWidget);
        ElaToggleSwitch *savePackLogSwitch = new ElaToggleSwitch(savePackLogWidget);
        Utils::setWidgetPixelSize(savePackLogSwitch, 12);
        savePackLogLayout->addWidget(savePackLogText);
        savePackLogLayout->addStretch();
        savePackLogLayout->addWidget(savePackLogSwitch);
        generalLayout->addWidget(savePackLogWidget);

        // language
        ElaScrollPageArea *languageWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *languageLayout = new QHBoxLayout(languageWidget);
        ElaText *languageText = new ElaText(tr("语言"), 12, languageWidget);
        ElaComboBox *languageComboBox = new ElaComboBox(languageWidget);
        languageComboBox->installEventFilter(this);
        languageComboBox->addItem("简体中文");
        languageComboBox->addItem("English");
        languageLayout->addWidget(languageText);
        languageLayout->addStretch();
        languageLayout->addWidget(languageComboBox);
        generalLayout->addWidget(languageWidget);

        // console input encoding
        ElaScrollPageArea *consoleInputEncodingWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *consoleInputEncodingLayout = new QHBoxLayout(consoleInputEncodingWidget);
        ElaText *consoleInputEncodingText = new ElaText(tr("控制台输入编码"), 12, consoleInputEncodingWidget);
        ElaComboBox *consoleInputEncodingComboBox = new ElaComboBox(consoleInputEncodingWidget);
        consoleInputEncodingComboBox->installEventFilter(this);
        consoleInputEncodingComboBox->addItem("utf-8");
        consoleInputEncodingComboBox->addItem("gbk");
        consoleInputEncodingComboBox->addItem("ascii");
        consoleInputEncodingLayout->addWidget(consoleInputEncodingText);
        consoleInputEncodingLayout->addStretch();
        consoleInputEncodingLayout->addWidget(consoleInputEncodingComboBox);
        generalLayout->addWidget(consoleInputEncodingWidget);

        // console output encoding
        ElaScrollPageArea *consoleOutputEncodingWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *consoleOutputEncodingLayout = new QHBoxLayout(consoleOutputEncodingWidget);
        ElaText *consoleOutputEncodingText = new ElaText(tr("控制台输出编码"), 12, consoleOutputEncodingWidget);
        ElaComboBox *consoleOutputEncodingComboBox = new ElaComboBox(consoleOutputEncodingWidget);
        consoleOutputEncodingComboBox->installEventFilter(this);
        consoleOutputEncodingComboBox->addItem("utf-8");
        consoleOutputEncodingComboBox->addItem("gbk");
        consoleOutputEncodingComboBox->addItem("ascii");
        consoleOutputEncodingLayout->addWidget(consoleOutputEncodingText);
        consoleOutputEncodingLayout->addStretch();
        consoleOutputEncodingLayout->addWidget(consoleOutputEncodingComboBox);
        generalLayout->addWidget(consoleOutputEncodingWidget);

        // pack timer trigger interval(PTTI)
        ElaScrollPageArea *PTTIWidget = new ElaScrollPageArea(generalCard);
        QHBoxLayout *PTTILayout = new QHBoxLayout(PTTIWidget);
        ElaText *PTTIText = new ElaText(tr("打包计时器触发间隔(ms)"), 12, PTTIWidget);
        PTTIText->setFixedWidth(200);
        ElaSpinBox *PTTISpinBox = new ElaSpinBox(PTTIWidget);
        PTTISpinBox->installEventFilter(this);
        PTTISpinBox->setMaximum(INT_MAX);
        PTTISpinBox->setMinimum(1);
        PTTISpinBox->setButtonMode(ElaSpinBoxType::Compact);
        PTTILayout->addWidget(PTTIText);
        PTTILayout->addStretch();
        PTTILayout->addWidget(PTTISpinBox);
        generalLayout->addWidget(PTTIWidget);

        scrollLayout->addWidget(generalCard);

        // Connect signals and slots
        // window
        connect(showExitConfirmSwitch, &ElaToggleSwitch::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsShowCloseWindow, checked);
            hideOnExitWidget->setVisible(!checked);
        });
        connect(hideOnExitSwitch, &ElaToggleSwitch::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsHideOnClose, checked);
        });
        connect(splashScreenSwitch, &ElaToggleSwitch::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsSplashScreen, checked);
        });
        connect(maxPackLogCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
            config.setInt(ConfigItem::MaxPackLogCount, value);
        });
        connect(savePackLogSwitch, &ElaToggleSwitch::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsSavePackLog, checked);
        });
        // language
        connect(languageComboBox, QOverload<int>::of(&QComboBox::activated), [=](int index) {
            Language language = Utils::intToEnum<Language>(index);
            if (language != config.getLanguage(ConfigItem::Language)) {
                nApp.changeLanguage(language);
            }
        });
        // console
        connect(consoleInputEncodingComboBox, QOverload<int>::of(&QComboBox::activated), [=](int index) {
            Encoding encoding = Utils::intToEnum<Encoding>(index);
            config.setEncodingEnum(ConfigItem::ConsoleInputEncoding, encoding);
        });
        connect(consoleOutputEncodingComboBox, QOverload<int>::of(&QComboBox::activated), [=](int index) {
            Encoding encoding = Utils::intToEnum<Encoding>(index);
            config.setEncodingEnum(ConfigItem::ConsoleOutputEncoding, encoding);
        });
        // PTTI
        connect(PTTISpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
            config.setInt(ConfigItem::PackTimerTriggerInterval, value);
        });

        // Update UI
        connect(&UpdateClock::instance(), &UpdateClock::updateUI, this, [=]() {
            showExitConfirmSwitch->setIsToggled(config.getBool(ConfigItem::IsShowCloseWindow, ConfigGroup::General));
            hideOnExitSwitch->setIsToggled(config.getBool(ConfigItem::IsHideOnClose, ConfigGroup::General));
            splashScreenSwitch->setIsToggled(config.getBool(ConfigItem::IsSplashScreen, ConfigGroup::General));
            maxPackLogCountSpinBox->setValue(config.getInt(ConfigItem::MaxPackLogCount, ConfigGroup::General));
            savePackLogSwitch->setIsToggled(config.getBool(ConfigItem::IsSavePackLog, ConfigGroup::General));
            languageComboBox->setCurrentIndex(
                Utils::enumToInt<Language>(config.getLanguage(ConfigItem::Language, ConfigGroup::General)));
            consoleInputEncodingComboBox->setCurrentIndex(
                Utils::enumToInt<Encoding>(
                    config.getEncodingEnum(ConfigItem::ConsoleInputEncoding, ConfigGroup::General)));
            consoleOutputEncodingComboBox->setCurrentIndex(
                Utils::enumToInt<Encoding>(
                    config.getEncodingEnum(ConfigItem::ConsoleOutputEncoding, ConfigGroup::General)));
            PTTISpinBox->setValue(config.getInt(ConfigItem::PackTimerTriggerInterval, ConfigGroup::General));
        });
    }
#pragma endregion

    // Appearance
#pragma region Appearance
    {
        this->appearanceCard = new QWidget(scrollWidget);
        appearanceCard->setFixedHeight(QWIDGETSIZE_MAX);
        appearanceCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *appearanceLayout = new QVBoxLayout(appearanceCard);
        appearanceLayout->setSpacing(18);
        appearanceLayout->setContentsMargins(0, 0, 0, 0);
        appearanceLayout->setAlignment(Qt::AlignTop);
        appearanceLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("外观设置"), 16, appearanceCard);
        appearanceLayout->addWidget(title);

        // theme
        ElaScrollPageArea *themeWidget = new ElaScrollPageArea(appearanceCard);
        QHBoxLayout *themeLayout = new QHBoxLayout(themeWidget);
        ElaText *themeText = new ElaText(tr("主题"), 12, themeWidget);
        ElaComboBox *themeComboBox = new ElaComboBox(themeWidget);
        themeComboBox->addItems(QStringList() << "Light" << "Dark");
        themeComboBox->installEventFilter(this);
        themeLayout->addWidget(themeText);
        themeLayout->addStretch();
        themeLayout->addWidget(themeComboBox);
        appearanceLayout->addWidget(themeWidget);

        // display mode
        ElaScrollPageArea *displayModeWidget = new ElaScrollPageArea(appearanceCard);
        QHBoxLayout *displayModeLayout = new QHBoxLayout(displayModeWidget);
        ElaText *displayModeText = new ElaText(tr("窗口效果"), 12, displayModeWidget);
        ElaRadioButton *normalButton = new ElaRadioButton("Normal", displayModeWidget);
        ElaRadioButton *elaMicaButton = new ElaRadioButton("Ela Mica", displayModeWidget);
        Utils::setWidgetPixelSize(normalButton, 12);
        Utils::setWidgetPixelSize(elaMicaButton, 12);
        displayModeLayout->addWidget(displayModeText);
        displayModeLayout->addStretch();
        displayModeLayout->addWidget(normalButton);
        displayModeLayout->addWidget(elaMicaButton);
#ifdef Q_OS_WIN
        ElaRadioButton *micaButton = new ElaRadioButton("Mica", displayModeWidget);
        ElaRadioButton *micaAltButton = new ElaRadioButton(tr("Mica-Alt"), displayModeWidget);
        ElaRadioButton *acrylicButton = new ElaRadioButton("Acrylic", this);
        ElaRadioButton *dwmBlurnormalButton = new ElaRadioButton("Dwm-Blur", this);
        Utils::setWidgetPixelSize(micaButton, 12);
        Utils::setWidgetPixelSize(micaAltButton, 12);
        Utils::setWidgetPixelSize(acrylicButton, 12);
        Utils::setWidgetPixelSize(dwmBlurnormalButton, 12);
        displayModeLayout->addWidget(micaButton);
        displayModeLayout->addWidget(micaAltButton);
        displayModeLayout->addWidget(acrylicButton);
        displayModeLayout->addWidget(dwmBlurnormalButton);
#endif
        normalButton->setChecked(true);
        QButtonGroup *displayButtonGroup = new QButtonGroup(displayModeWidget);
        displayButtonGroup->addButton(normalButton, 0);
        displayButtonGroup->addButton(elaMicaButton, 1);
#ifdef Q_OS_WIN
        displayButtonGroup->addButton(micaButton, 2);
        displayButtonGroup->addButton(micaAltButton, 3);
        displayButtonGroup->addButton(acrylicButton, 4);
        displayButtonGroup->addButton(dwmBlurnormalButton, 5);
#endif
        appearanceLayout->addWidget(displayModeWidget);

        // window background
        ElaScrollPageArea *windowBackgroundWidget = new ElaScrollPageArea(appearanceCard);
        QHBoxLayout *windowBackgroundLayout = new QHBoxLayout(windowBackgroundWidget);
        ElaText *windowBgText = new ElaText(tr("背景"), 12, windowBackgroundWidget);
        ElaRadioButton *windowBgNormalButton = new ElaRadioButton(tr("正常"), windowBackgroundWidget);
        ElaRadioButton *windowBgPixmapButton = new ElaRadioButton(tr("静态图片"), windowBackgroundWidget);
        ElaRadioButton *windowBgMovieButton = new ElaRadioButton(tr("动态图片"), windowBackgroundWidget);
        Utils::setWidgetPixelSize(windowBgNormalButton, 12);
        Utils::setWidgetPixelSize(windowBgPixmapButton, 12);
        Utils::setWidgetPixelSize(windowBgMovieButton, 12);
        windowBackgroundLayout->addWidget(windowBgText);
        windowBackgroundLayout->addStretch();
        windowBackgroundLayout->addWidget(windowBgNormalButton);
        windowBackgroundLayout->addWidget(windowBgPixmapButton);
        windowBackgroundLayout->addWidget(windowBgMovieButton);

        QButtonGroup *windowBgButtonGroup = new QButtonGroup(windowBackgroundWidget);
        windowBgButtonGroup->addButton(windowBgNormalButton, 0);
        windowBgButtonGroup->addButton(windowBgPixmapButton, 1);
        windowBgButtonGroup->addButton(windowBgMovieButton, 2);
        appearanceLayout->addWidget(windowBackgroundWidget);

        // pixmap path
        ElaScrollPageArea *pixmapWidget = new ElaScrollPageArea(appearanceCard);
        QHBoxLayout *pixmapLayout = new QHBoxLayout(pixmapWidget);
        ElaText *pixmapText = new ElaText(tr("静态图片路径"), 12, pixmapWidget);
        ElaLineEdit *pixmapEdit = new ElaLineEdit(pixmapWidget);
        ElaPushButton *pixmapButton = new ElaPushButton(tr("浏览"), pixmapWidget);
        Utils::setWidgetPixelSize(pixmapButton, 12);
        pixmapLayout->addWidget(pixmapText);
        pixmapLayout->addWidget(pixmapEdit);
        pixmapLayout->addWidget(pixmapButton);
        appearanceLayout->addWidget(pixmapWidget);

        // movie path
        ElaScrollPageArea *movieWidget = new ElaScrollPageArea(appearanceCard);
        QHBoxLayout *movieLayout = new QHBoxLayout(movieWidget);
        ElaText *movieText = new ElaText(tr("动态图片路径"), 12, movieWidget);
        ElaLineEdit *movieEdit = new ElaLineEdit(movieWidget);
        ElaPushButton *movieButton = new ElaPushButton(tr("浏览"), movieWidget);
        Utils::setWidgetPixelSize(movieButton, 12);
        movieLayout->addWidget(movieText);
        movieLayout->addWidget(movieEdit);
        movieLayout->addWidget(movieButton);
        appearanceLayout->addWidget(movieWidget);

        scrollLayout->addWidget(appearanceCard);

        // Connect signals and slots
        // theme
        connect(themeComboBox, QOverload<int>::of(&QComboBox::activated), [&](int index) {
            Theme theme = Utils::intToEnum<Theme>(index);
            config.setTheme(ConfigItem::Theme, theme, ConfigGroup::Appearance);
            if (theme == Theme::Light) {
                eTheme->setThemeMode(ElaThemeType::Light);
            } else if (theme == Theme::Dark) {
                eTheme->setThemeMode(ElaThemeType::Dark);
            }
        });
        // display mode
        connect(displayButtonGroup, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this,
                [=](QAbstractButton *button, bool isToggled) {
                    if (isToggled) {
                        eApp->setWindowDisplayMode(
                            static_cast<ElaApplicationType::WindowDisplayMode>(displayButtonGroup->id(button)));
                        config.setWindowDisplayMode(ConfigItem::WindowDisplayMode,
                                                    static_cast<WindowDisplayMode>(displayButtonGroup->id(button)),
                                                    ConfigGroup::Appearance);
                    }
                });
        connect(eApp, &ElaApplication::pWindowDisplayModeChanged, this, [=]() {
            auto button = displayButtonGroup->button(eApp->getWindowDisplayMode());
            ElaRadioButton *elaRadioButton = dynamic_cast<ElaRadioButton *>(button);
            if (elaRadioButton) {
                elaRadioButton->setChecked(true);
            }
        });
        // window bg
        connect(windowBgButtonGroup, QOverload<QAbstractButton *, bool>::of(&QButtonGroup::buttonToggled), this, [=
                ](QAbstractButton *button, bool isToggled) {
                    if (isToggled) {
                        window->setWindowPaintMode(static_cast<ElaWindowType::PaintMode>(windowBgButtonGroup->id(button)));
                        config.setWindowBackground(ConfigItem::WindowBackground,
                            static_cast<WindowBackground>(windowBgButtonGroup->id(button)), ConfigGroup::Appearance);
                    }
                });
        connect(window, &ElaWindow::pWindowPaintModeChanged, this, [=]() {

        });

        // pixmap path
        connect(pixmapEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
            config.setString(ConfigItem::PixmapPath, text, ConfigGroup::Appearance);
            window->setWindowPixmap(ElaThemeType::Light, QPixmap(text));
            window->setWindowPixmap(ElaThemeType::Dark, QPixmap(text));
        });
        // movie path
        connect(movieEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
            config.setString(ConfigItem::MoviePath, text, ConfigGroup::Appearance);
            window->setWindowMoviePath(ElaThemeType::Light, text);
            window->setWindowMoviePath(ElaThemeType::Dark, text);
        });
        // pixmap button
        connect(pixmapButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getOpenFileName(this, "Nuitka Studio",
                config.getString(ConfigItem::DefaultDataPath), "Pixmap (*.png *.jpg *.jpeg);;All files(*)");
            pixmapEdit->setText(path);
            config.setString(ConfigItem::PixmapPath, path, ConfigGroup::Appearance);
            window->setWindowPixmap(ElaThemeType::Light, QPixmap(path));
            window->setWindowPixmap(ElaThemeType::Dark, QPixmap(path));
        });
        // movie button
        connect(movieButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getOpenFileName(this, "Nuitka Studio",
                config.getString(ConfigItem::DefaultDataPath), "Movie file (*.gif *.apng *.png);;All files(*)");
            movieEdit->setText(path);
            config.setString(ConfigItem::MoviePath, path, ConfigGroup::Appearance);
            window->setWindowMoviePath(ElaThemeType::Light, path);
            window->setWindowMoviePath(ElaThemeType::Dark, path);
        });

        // update
        connect(&UpdateClock::instance(), &UpdateClock::updateUI, this, [=]() {
            themeComboBox->setCurrentIndex(Utils::enumToInt<Theme>
                (config.getTheme(ConfigItem::Theme, ConfigGroup::Appearance)));

            auto displayButton = displayButtonGroup->button(eApp->getWindowDisplayMode());
            ElaRadioButton *elaDisplayRadioButton = dynamic_cast<ElaRadioButton *>(displayButton);
            if (elaDisplayRadioButton) {
                elaDisplayRadioButton->setChecked(true);
            }

            auto backgroundButton = windowBgButtonGroup->button(window->getWindowPaintMode());
            ElaRadioButton *elaBackgroundRadioButton = dynamic_cast<ElaRadioButton *>(backgroundButton);
            if (elaBackgroundRadioButton) {
                elaBackgroundRadioButton->setChecked(true);
            }

            pixmapEdit->setText(config.getString(ConfigItem::PixmapPath, ConfigGroup::Appearance));
            movieEdit->setText(config.getString(ConfigItem::MoviePath, ConfigGroup::Appearance));
        });
    }
#pragma endregion

    // Default Path Settings
#pragma region DefaultPathCardSection
    {
        this->defaultPathCard = new QWidget(scrollWidget);
        defaultPathCard->setFixedHeight(QWIDGETSIZE_MAX);
        defaultPathCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *defaultPathLayout = new QVBoxLayout(defaultPathCard);
        defaultPathLayout->setSpacing(18);
        defaultPathLayout->setContentsMargins(0, 0, 0, 0);
        defaultPathLayout->setAlignment(Qt::AlignTop);
        defaultPathLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("默认路径设置"), 16, defaultPathCard);
        defaultPathLayout->addWidget(title);

        // python path
        ElaScrollPageArea *pythonWidget = new ElaScrollPageArea(defaultPathCard);
        QHBoxLayout *pythonLayout = new QHBoxLayout(pythonWidget);
        ElaText *pythonText = new ElaText(tr("默认 Python 路径"), 12, pythonWidget);
        ElaLineEdit *pythonEdit = new ElaLineEdit(pythonWidget);
        ElaPushButton *pythonButton = new ElaPushButton(tr("浏览"), pythonWidget);
        Utils::setWidgetPixelSize(pythonButton, 12);
        pythonLayout->addWidget(pythonText);
        pythonLayout->addWidget(pythonEdit);
        pythonLayout->addWidget(pythonButton);
        defaultPathLayout->addWidget(pythonWidget);

        // project path
        ElaScrollPageArea *projectWidget = new ElaScrollPageArea(defaultPathCard);
        QHBoxLayout *projectLayout = new QHBoxLayout(projectWidget);
        ElaText *projectText = new ElaText(tr("默认项目路径"), 12, projectWidget);
        ElaLineEdit *projectEdit = new ElaLineEdit(projectWidget);
        ElaPushButton *projectButton = new ElaPushButton(tr("浏览"), projectWidget);
        Utils::setWidgetPixelSize(projectButton, 12);
        projectLayout->addWidget(projectText);
        projectLayout->addWidget(projectEdit);
        projectLayout->addWidget(projectButton);
        defaultPathLayout->addWidget(projectWidget);

        // data path
        ElaScrollPageArea *dataWidget = new ElaScrollPageArea(defaultPathCard);
        QHBoxLayout *dataLayout = new QHBoxLayout(dataWidget);
        ElaText *dataText = new ElaText(tr("默认数据路径"), 12, dataWidget);
        ElaLineEdit *dataEdit = new ElaLineEdit(dataWidget);
        ElaPushButton *dataButton = new ElaPushButton(tr("浏览"), dataWidget);
        Utils::setWidgetPixelSize(dataButton, 12);
        dataLayout->addWidget(dataText);
        dataLayout->addWidget(dataEdit);
        dataLayout->addWidget(dataButton);
        defaultPathLayout->addWidget(dataWidget);

        // Connect signals and slots
        connect(pythonEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
            config.setString(ConfigItem::DefaultPythonPath, text);
        });
        connect(projectEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
            config.setString(ConfigItem::DefaultProjectPath, text);
        });
        connect(dataEdit, &QLineEdit::textChanged, this, [=](const QString &text) {
            config.setString(ConfigItem::DefaultDataPath, text);
        });
        connect(pythonButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getExistingDirectory(this, "Nuitka Studio",
                                                             config.getString(ConfigItem::DefaultPythonPath));
            pythonEdit->setText(path);
            config.setString(ConfigItem::DefaultPythonPath, path);
        });
        connect(projectButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getExistingDirectory(this, "Nuitka Studio",
                                                             config.getString(ConfigItem::DefaultProjectPath));
            projectButton->setText(path);
            config.setString(ConfigItem::DefaultProjectPath, path);
        });
        connect(dataButton, &QPushButton::clicked, this, [=]() {
            QString path = QFileDialog::getExistingDirectory(this, "Nuitka Studio",
                                                             config.getString(ConfigItem::DefaultDataPath));
            dataButton->setText(path);
            config.setString(ConfigItem::DefaultDataPath, path);
        });

        connect(&UpdateClock::instance(), &UpdateClock::updateUI, this, [=]() {
            pythonEdit->setText(config.getString(ConfigItem::DefaultPythonPath, ConfigGroup::DefaultPath));
            projectEdit->setText(config.getString(ConfigItem::DefaultProjectPath, ConfigGroup::DefaultPath));
            dataEdit->setText(config.getString(ConfigItem::DefaultDataPath, ConfigGroup::DefaultPath));
        });

        scrollLayout->addWidget(defaultPathCard);
    }
#pragma endregion

    // scroll area config
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);

    // main layout
    mainLayout->addWidget(scrollArea);
}

bool SettingsPage::eventFilter(QObject *watched, QEvent *event) {
    if (qobject_cast<QComboBox *>(watched) && event->type() == QEvent::Wheel) {
        return true;
    }
    if (qobject_cast<QSpinBox *>(watched) && event->type() == QEvent::Wheel) {
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void SettingsPage::scrollTo(PageCard card) const {
    switch (card) {
        case PageCard::SettingsPageGeneralCard:
            this->scrollArea->ensureWidgetVisible(this->generalCard);
            break;
        case PageCard::SettingsPageAppearanceCard:
            this->scrollArea->ensureWidgetVisible(this->appearanceCard);
            break;
        case PageCard::SettingsPageDefaultPathCard:
            this->scrollArea->ensureWidgetVisible(this->defaultPathCard);
            break;
        default:
            Logger::error("SettingsPage::scrollTo: invalid card");
            break;
    }
}
