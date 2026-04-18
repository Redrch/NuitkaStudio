//
// Created by redrch on 2026/4/12.
//

#include "settings_page.h"
#include "../utils/utils.h"
#include <QVBoxLayout>
#include <ElaScrollArea.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>
#include <ElaComboBox.h>
#include <ElaSpinBox.h>
#include <ElaCheckBox.h>

SettingsPage::SettingsPage(QWidget *parent) : QWidget(parent) {
    this->setMinimumSize(950, 570);
    // main layout
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    // scroll area
    ElaScrollArea *scrollArea = new ElaScrollArea(this);
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
        ElaScrollPageArea *generalCard = new ElaScrollPageArea(scrollWidget);
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

        // language
        QWidget *languageWidget = new QWidget(generalCard);
        QHBoxLayout *languageLayout = new QHBoxLayout(languageWidget);
        ElaText *languageText = new ElaText(tr("语言"), 12, languageWidget);
        languageText->setFixedWidth(labelWidget);
        ElaComboBox *languageComboBox = new ElaComboBox(languageWidget);
        languageComboBox->addItem("简体中文");
        languageComboBox->addItem("English");
        languageLayout->addWidget(languageText);
        languageLayout->addWidget(languageComboBox, 1);
        generalLayout->addWidget(languageWidget);

        // window
        QWidget *windowWidget = new QWidget(generalCard);
        QHBoxLayout *windowLayout = new QHBoxLayout(windowWidget);
        ElaText *windowText = new ElaText(tr("窗口设置"), 12, windowWidget);
        windowText->setFixedWidth(labelWidget + 10);
        ElaCheckBox *showExitConfirmCheckBox = new ElaCheckBox(tr("退出确认提示"), windowWidget);
        Utils::setWidgetPixelSize(showExitConfirmCheckBox, 12);
        ElaCheckBox *hideOnExitCheckBox = new ElaCheckBox(tr("退出时最小化到系统托盘"), windowWidget);
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
        maxPackLogCountSpinBox->setButtonMode(ElaSpinBoxType::Compact);
        ElaCheckBox *savePackLogCheckBox = new ElaCheckBox(tr("保存打包日志"), npfWidget);
        Utils::setWidgetPixelSize(savePackLogCheckBox, 12);
        npfLayout->addWidget(npfText);
        npfLayout->addWidget(maxPackLogCountText);
        npfLayout->addWidget(maxPackLogCountSpinBox);
        npfLayout->addStretch();
        npfLayout->addWidget(savePackLogCheckBox);
        generalLayout->addWidget(npfWidget);

        // console input encoding
        QWidget *consoleInputEncodingWidget = new QWidget(generalCard);
        QHBoxLayout *consoleInputEncodingLayout = new QHBoxLayout(consoleInputEncodingWidget);
        ElaText *consoleInputEncodingText = new ElaText(tr("控制台输入编码"), 12, consoleInputEncodingWidget);
        consoleInputEncodingText->setFixedWidth(labelWidget);
        ElaComboBox *consoleInputEncodingComboBox = new ElaComboBox(consoleInputEncodingWidget);
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
        PTTISpinBox->setMaximum(INT_MAX);
        PTTISpinBox->setMinimum(1);
        PTTISpinBox->setButtonMode(ElaSpinBoxType::Compact);
        PTTILayout->addWidget(PTTIText);
        PTTILayout->addWidget(PTTISpinBox);
        PTTILayout->addStretch();
        generalLayout->addWidget(PTTIWidget);

        scrollLayout->addWidget(generalCard);

        // Connect signals and slots
        connect(languageComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index) {

        });
    }
#pragma endregion

    // Default Path Settings
#pragma region DefaultPathCardSection
{
    ElaScrollPageArea *defaultPathCard = new ElaScrollPageArea(scrollWidget);
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


