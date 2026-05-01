//
// Created by redrch on 2026/4/12.
//

#include "settings_page.h"
#include "global/application.h"
#include "global/update_clock.h"
#include "utils/utils.h"

#include <QVBoxLayout>
#include <ElaScrollArea.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>
#include <ElaComboBox.h>
#include <ElaSpinBox.h>
#include <ElaCheckBox.h>
#include <ElaLineEdit.h>

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

    // General Settings
#pragma region GeneralCardSection
    {
        this->generalCard = new ElaScrollPageArea(scrollWidget);
        generalCard->setFixedHeight(QWIDGETSIZE_MAX);
        generalCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *generalLayout = new QVBoxLayout(generalCard);
        generalLayout->setSpacing(18);
        generalLayout->setContentsMargins(20, 20, 20, 20);
        generalLayout->setAlignment(Qt::AlignTop);
        generalLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("通用设置"), 16, generalCard);
        generalLayout->addWidget(title);

        int labelWidget = 150;

        // window
        QWidget *windowWidget = new QWidget(generalCard);
        QHBoxLayout *windowLayout = new QHBoxLayout(windowWidget);
        ElaText *windowText = new ElaText(tr("窗口设置"), 12, windowWidget);
        windowText->setFixedWidth(labelWidget + 10);
        ElaCheckBox *showExitConfirmCheckBox = new ElaCheckBox(tr("退出确认提示"), windowWidget);
        Utils::setWidgetPixelSize(showExitConfirmCheckBox, 12);
        ElaCheckBox *hideOnExitCheckBox = new ElaCheckBox(tr("退出时最小化到系统托盘"), windowWidget);
        hideOnExitCheckBox->setEnabled(false);
        Utils::setWidgetPixelSize(hideOnExitCheckBox, 12);
        ElaCheckBox *splashScreenCheckBox = new ElaCheckBox(tr("开屏动画"), windowWidget);
        Utils::setWidgetPixelSize(splashScreenCheckBox, 12);
        windowLayout->addWidget(windowText);
        windowLayout->addWidget(showExitConfirmCheckBox);
        windowLayout->addStretch();
        windowLayout->addWidget(hideOnExitCheckBox);
        windowLayout->addStretch();
        windowLayout->addWidget(splashScreenCheckBox);
        generalLayout->addWidget(windowWidget);

        // npf file
        QWidget *npfWidget = new QWidget(generalCard);
        QHBoxLayout *npfLayout = new QHBoxLayout(npfWidget);
        ElaText *npfText = new ElaText(tr("NPF设置"), 12, npfWidget);
        npfText->setFixedWidth(labelWidget + 10);
        ElaText *maxPackLogCountText = new ElaText(tr("最大日志数量"), 12, npfWidget);
        ElaSpinBox *maxPackLogCountSpinBox = new ElaSpinBox(npfWidget);
        maxPackLogCountSpinBox->installEventFilter(this);
        maxPackLogCountSpinBox->setButtonMode(ElaSpinBoxType::Compact);
        ElaCheckBox *savePackLogCheckBox = new ElaCheckBox(tr("保存打包日志"), npfWidget);
        Utils::setWidgetPixelSize(savePackLogCheckBox, 12);
        npfLayout->addWidget(npfText);
        npfLayout->addWidget(maxPackLogCountText);
        npfLayout->addWidget(maxPackLogCountSpinBox);
        npfLayout->addStretch();
        npfLayout->addWidget(savePackLogCheckBox);
        generalLayout->addWidget(npfWidget);

        // language
        QWidget *languageWidget = new QWidget(generalCard);
        QHBoxLayout *languageLayout = new QHBoxLayout(languageWidget);
        ElaText *languageText = new ElaText(tr("语言"), 12, languageWidget);
        languageText->setFixedWidth(labelWidget);
        ElaComboBox *languageComboBox = new ElaComboBox(languageWidget);
        languageComboBox->installEventFilter(this);
        languageComboBox->addItem("简体中文");
        languageComboBox->addItem("English");
        languageLayout->addWidget(languageText);
        languageLayout->addWidget(languageComboBox, 1);
        generalLayout->addWidget(languageWidget);

        // console input encoding
        QWidget *consoleInputEncodingWidget = new QWidget(generalCard);
        QHBoxLayout *consoleInputEncodingLayout = new QHBoxLayout(consoleInputEncodingWidget);
        ElaText *consoleInputEncodingText = new ElaText(tr("控制台输入编码"), 12, consoleInputEncodingWidget);
        consoleInputEncodingText->setFixedWidth(labelWidget);
        ElaComboBox *consoleInputEncodingComboBox = new ElaComboBox(consoleInputEncodingWidget);
        consoleInputEncodingComboBox->installEventFilter(this);
        consoleInputEncodingComboBox->addItem("utf-8");
        consoleInputEncodingComboBox->addItem("gbk");
        consoleInputEncodingComboBox->addItem("ascii");
        consoleInputEncodingLayout->addWidget(consoleInputEncodingText);
        consoleInputEncodingLayout->addWidget(consoleInputEncodingComboBox, 1);
        generalLayout->addWidget(consoleInputEncodingWidget);

        // console output encoding
        QWidget *consoleOutputEncodingWidget = new QWidget(generalCard);
        QHBoxLayout *consoleOutputEncodingLayout = new QHBoxLayout(consoleOutputEncodingWidget);
        ElaText *consoleOutputEncodingText = new ElaText(tr("控制台输出编码"), 12, consoleOutputEncodingWidget);
        consoleOutputEncodingText->setFixedWidth(labelWidget);
        ElaComboBox *consoleOutputEncodingComboBox = new ElaComboBox(consoleOutputEncodingWidget);
        consoleOutputEncodingComboBox->installEventFilter(this);
        consoleOutputEncodingComboBox->addItem("utf-8");
        consoleOutputEncodingComboBox->addItem("gbk");
        consoleOutputEncodingComboBox->addItem("ascii");
        consoleOutputEncodingLayout->addWidget(consoleOutputEncodingText);
        consoleOutputEncodingLayout->addWidget(consoleOutputEncodingComboBox, 1);
        generalLayout->addWidget(consoleOutputEncodingWidget);

        // pack timer trigger interval(PTTI)
        QWidget *PTTIWidget = new QWidget(generalCard);
        QHBoxLayout *PTTILayout = new QHBoxLayout(PTTIWidget);
        ElaText *PTTIText = new ElaText(tr("打包计时器触发间隔(ms)"), 12, PTTIWidget);
        PTTIText->setFixedWidth(labelWidget);
        ElaSpinBox *PTTISpinBox = new ElaSpinBox(PTTIWidget);
        PTTISpinBox->installEventFilter(this);
        PTTISpinBox->setMaximum(INT_MAX);
        PTTISpinBox->setMinimum(1);
        PTTISpinBox->setButtonMode(ElaSpinBoxType::Compact);
        PTTILayout->addWidget(PTTIText);
        PTTILayout->addWidget(PTTISpinBox);
        PTTILayout->addStretch();
        generalLayout->addWidget(PTTIWidget);

        scrollLayout->addWidget(generalCard);

        // Connect signals and slots
        // window
        connect(showExitConfirmCheckBox, &QCheckBox::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsShowCloseWindow, checked);
            hideOnExitCheckBox->setEnabled(!checked);
        });
        connect(hideOnExitCheckBox, &QCheckBox::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsHideOnClose, checked);
        });
        connect(splashScreenCheckBox, &QCheckBox::toggled, [=](bool checked) {
            config.setBool(ConfigItem::IsSplashScreen, checked);
        });
        // npf file
        connect(maxPackLogCountSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), [=](int value) {
            config.setInt(ConfigItem::MaxPackLogCount, value);
        });
        connect(savePackLogCheckBox, &QCheckBox::toggled, [=](bool checked) {
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
            showExitConfirmCheckBox->setChecked(config.getBool(ConfigItem::IsShowCloseWindow));
            hideOnExitCheckBox->setChecked(config.getBool(ConfigItem::IsHideOnClose));
            splashScreenCheckBox->setChecked(config.getBool(ConfigItem::IsSplashScreen));
            maxPackLogCountSpinBox->setValue(config.getInt(ConfigItem::MaxPackLogCount));
            savePackLogCheckBox->setChecked(config.getBool(ConfigItem::IsSavePackLog));
            languageComboBox->setCurrentIndex(
                Utils::enumToInt<Language>(config.getLanguage(ConfigItem::Language)));
            consoleInputEncodingComboBox->setCurrentIndex(
                Utils::enumToInt<Encoding>(config.getEncodingEnum(ConfigItem::ConsoleInputEncoding)));
            consoleOutputEncodingComboBox->setCurrentIndex(
                Utils::enumToInt<Encoding>(config.getEncodingEnum(ConfigItem::ConsoleOutputEncoding)));
            PTTISpinBox->setValue(config.getInt(ConfigItem::PackTimerTriggerInterval));
        });
        connect(ElaTheme::getInstance(), &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode mode) {
            const QColor &textColor = ElaThemeColor(mode, ThemeColor::BasicText);
            QString textColorHex = textColor.name();
            QString textStyleSheet = QString("color: %1;").arg(textColorHex);
            Utils::addWidgetStyleSheet(showExitConfirmCheckBox, textStyleSheet);
            Utils::addWidgetStyleSheet(hideOnExitCheckBox,
                QString("color: %1;").arg(ElaThemeColor(mode, ThemeColor::BasicTextDisable).name()));
            Utils::addWidgetStyleSheet(splashScreenCheckBox, textStyleSheet);
            Utils::addWidgetStyleSheet(savePackLogCheckBox, textStyleSheet);
        });
    }
#pragma endregion

    // Default Path Settings
#pragma region DefaultPathCardSection
{
    this->defaultPathCard = new ElaScrollPageArea(scrollWidget);
    defaultPathCard->setFixedHeight(QWIDGETSIZE_MAX);
    defaultPathCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    QVBoxLayout *defaultPathLayout = new QVBoxLayout(defaultPathCard);
    defaultPathLayout->setSpacing(18);
    defaultPathLayout->setContentsMargins(20, 20, 20, 20);
    defaultPathLayout->setAlignment(Qt::AlignTop);
    defaultPathLayout->setSizeConstraint(QLayout::SetMinimumSize);

    // title
    ElaText *title = new ElaText(tr("默认路径设置"), 16, defaultPathCard);
    defaultPathLayout->addWidget(title);

    // python path
    QWidget *pythonWidget = new QWidget(defaultPathCard);
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
    QWidget *projectWidget = new QWidget(defaultPathCard);
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
    QWidget *dataWidget = new QWidget(defaultPathCard);
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
    if (qobject_cast<QComboBox*>(watched) && event->type() == QEvent::Wheel) {
        return true;
    }
    if (qobject_cast<QSpinBox*>(watched) && event->type() == QEvent::Wheel) {
        return true;
    }
    return QWidget::eventFilter(watched, event);
}

void SettingsPage::scrollTo(PageCard card) const {
    switch (card) {
        case PageCard::SettingsPageGeneralCard:
            this->scrollArea->ensureWidgetVisible(this->generalCard);
            break;
        case PageCard::SettingsPageDefaultPathCard:
            this->scrollArea->ensureWidgetVisible(this->defaultPathCard);
            break;
        default:
            Logger::error("SettingsPage::scrollTo: invalid card");
            break;
    }
}


