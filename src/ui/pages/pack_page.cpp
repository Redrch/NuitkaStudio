//
// Created by redrch on 2026/4/4.
//

#include "pack_page.h"
#include "utils/utils.h"
#include "common/simname.h"

#include <QHBoxLayout>
#include <QFileDialog>
#include <ElaText.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaCheckBox.h>
#include <ElaListView.h>
#include <ElaTheme.h>

PackPage::PackPage(QWidget *parent) : QWidget(parent) {
    this->setMinimumSize(600, 400);
    this->setStyleSheet("background: transparent;");

    this->assetListModel = new QStringListModel(this);
    this->isPacking = false;

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

    // add config row function
    auto addConfigRow = [&](QVBoxLayout *parentCardLayout, QWidget *parentWidget, const QString &labelText,
                            QWidget *edit, QWidget *button = nullptr, bool isStretch = true) {
        QHBoxLayout *rowLayout = new QHBoxLayout();
        ElaText *label = new ElaText(labelText, 12, parentWidget);
        label->setFixedWidth(80);
        rowLayout->addWidget(label);
        rowLayout->addWidget(edit);
        if (button) {
            rowLayout->addWidget(button);
        } else {
            if (isStretch) rowLayout->addStretch();
        }
        parentCardLayout->addLayout(rowLayout);
    };

    // Base card
#pragma region BaseCardSection
    {
        this->baseCard = new ElaScrollPageArea(scrollWidget);
        baseCard->setFixedHeight(QWIDGETSIZE_MAX);
        baseCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *baseLayout = new QVBoxLayout(baseCard);
        baseLayout->setSpacing(18);
        baseLayout->setContentsMargins(20, 20, 20, 20);
        baseLayout->setAlignment(Qt::AlignTop);
        baseLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *baseCardTitle = new ElaText(tr("基础设置"), 16, baseCard);
        baseLayout->addWidget(baseCardTitle);

        // project path
        ElaLineEdit *projectPathEdit = new ElaLineEdit(baseCard);
        ElaPushButton *projectPathButton = new ElaPushButton(tr("浏览"), baseCard);
        Utils::setWidgetPixelSize(projectPathButton, 12);
        addConfigRow(baseLayout, baseCard, tr("项目路径"), projectPathEdit, projectPathButton);

        // project name
        ElaLineEdit *projectNameEdit = new ElaLineEdit(baseCard);
        addConfigRow(baseLayout, baseCard, tr("项目名称"), projectNameEdit);

        // main file path
        ElaLineEdit *mainFileEdit = new ElaLineEdit(baseCard);
        mainFileEdit->setToolTip(tr("通常为 main.py 或项目同名 .py 文件"));
        ElaPushButton *mainFileButton = new ElaPushButton(tr("浏览"), baseCard);
        Utils::setWidgetPixelSize(mainFileButton, 12);
        addConfigRow(baseLayout, baseCard, tr("主文件路径"), mainFileEdit, mainFileButton);

        // python path
        ElaLineEdit *pythonPathEdit = new ElaLineEdit(baseCard);
        ElaPushButton *pythonPathButton = new ElaPushButton(tr("浏览"), baseCard);
        Utils::setWidgetPixelSize(pythonPathButton, 12);
        addConfigRow(baseLayout, baseCard, tr("Python 路径"), pythonPathEdit, pythonPathButton);

        // output path
        ElaLineEdit *outputPathEdit = new ElaLineEdit(baseCard);
        ElaPushButton *outputPathButton = new ElaPushButton(tr("浏览"), baseCard);
        Utils::setWidgetPixelSize(outputPathButton, 12);
        addConfigRow(baseLayout, baseCard, tr("输出路径"), outputPathEdit, outputPathButton);

        // output name
        ElaLineEdit *outputNameEdit = new ElaLineEdit(baseCard);
        addConfigRow(baseLayout, baseCard, tr("输出文件名称"), outputNameEdit);

        scrollLayout->addWidget(baseCard);

        // connect signals and slots
        connect(projectPathEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::ProjectPath, text);
        });
        connect(projectNameEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::ProjectName, text);
        });
        connect(mainFileEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::MainfilePath, text);
        });
        connect(pythonPathEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::PythonPath, text);
        });
        connect(outputPathEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::OutputPath, text);
        });
        connect(outputNameEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::OutputFilename, text);
        });
        connect(projectPathButton, &QPushButton::clicked, [=]() {
            QString path = QFileDialog::getExistingDirectory(baseCard, "Nuitka Studio",
                                                             config.getString(ConfigItem::DefaultProjectPath));
            projectPathEdit->setText(path);
            PCM.set(PCE::ProjectPath, path);
        });
        connect(mainFileButton, &QPushButton::clicked, [=]() {
            QString path = QFileDialog::getOpenFileName(baseCard, "Nuitka Studio",
                                                        config.getString(ConfigItem::DefaultProjectPath),
                                                        "Python File(*.py)");
            pythonPathEdit->setText(path);
            PCM.set(PCE::MainfilePath, path);
        });
        connect(pythonPathButton, &QPushButton::clicked, [=]() {
            QString path = QFileDialog::getOpenFileName(baseCard, "Nuitka Studio",
                                                        config.getString(ConfigItem::DefaultPythonPath),
                                                        "exe(*.exe)");
            pythonPathEdit->setText(path);
            PCM.set(PCE::PythonPath, path);
        });
        connect(outputPathButton, &QPushButton::clicked, [=]() {
            QString path = QFileDialog::getExistingDirectory(baseCard, "Nuitka Studio",
                                                             config.getString(ConfigItem::DefaultProjectPath));
            outputPathEdit->setText(path);
            PCM.set(PCE::OutputPath, path);
        });

        connect(&PCM, &ProjectConfigManager::updateUI, [=]() {
            projectPathEdit->setText(PCM.getString(PCE::ProjectPath));
            projectNameEdit->setText(PCM.getString(PCE::ProjectName));
            mainFileEdit->setText(PCM.getString(PCE::MainfilePath));
            pythonPathEdit->setText(PCM.getString(PCE::PythonPath));
            outputPathEdit->setText(PCM.getString(PCE::OutputPath));
            outputNameEdit->setText(PCM.getString(PCE::OutputFilename));
        });
    }
#pragma endregion

    // Pack card
#pragma region PackCardSection
    {
        this->packCard = new ElaScrollPageArea(scrollWidget);
        packCard->setFixedHeight(QWIDGETSIZE_MAX);
        packCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *packLayout = new QVBoxLayout(packCard);
        packLayout->setSpacing(18);
        packLayout->setContentsMargins(20, 20, 20, 20);
        packLayout->setAlignment(Qt::AlignTop);
        packLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("打包设置"), 16, packCard);
        packLayout->addWidget(title);

        // output settings
        QWidget *outputWidget = new QWidget(packCard);
        QHBoxLayout *outputLayout = new QHBoxLayout(outputWidget);
        outputLayout->setContentsMargins(0, 0, 0, 0);
        ElaText *outputText = new ElaText(tr("输出设置"), 12, outputWidget);
        outputLayout->addWidget(outputText);

        // standalone
        ElaCheckBox *standaloneButton = new ElaCheckBox(tr("独立模式"), outputWidget);
        Utils::setWidgetPixelSize(standaloneButton, 12);
        outputLayout->addWidget(standaloneButton);

        // onefile
        ElaCheckBox *onefileButton = new ElaCheckBox(tr("单文件输出"), outputWidget);
        Utils::setWidgetPixelSize(onefileButton, 12);
        outputLayout->addWidget(onefileButton);

        // remove packing file
        ElaCheckBox *removePackFile = new ElaCheckBox(tr("删除打包临时文件"), outputWidget);
        Utils::setWidgetPixelSize(removePackFile, 12);
        outputLayout->addWidget(removePackFile);

        packLayout->addWidget(outputWidget);

        // lto
        QWidget *ltoWidget = new QWidget(packCard);
        QHBoxLayout *ltoLayout = new QHBoxLayout(ltoWidget);
        ltoLayout->setContentsMargins(0, 0, 0, 0);
        ElaText *ltoText = new ElaText(tr("LTO 模式"), 12, ltoWidget);
        ElaCheckBox *ltoAuto = new ElaCheckBox(tr("自动"), ltoWidget);
        ElaCheckBox *ltoYes = new ElaCheckBox(tr("启用"), ltoWidget);
        ElaCheckBox *ltoNo = new ElaCheckBox(tr("禁用"), ltoWidget);
        ltoAuto->setAutoExclusive(true);
        ltoYes->setAutoExclusive(true);
        ltoNo->setAutoExclusive(true);
        Utils::setWidgetPixelSize(ltoAuto, 12);
        Utils::setWidgetPixelSize(ltoYes, 12);
        Utils::setWidgetPixelSize(ltoNo, 12);

        ltoLayout->addWidget(ltoText);
        ltoLayout->addWidget(ltoAuto);
        ltoLayout->addWidget(ltoYes);
        ltoLayout->addWidget(ltoNo);

        packLayout->addWidget(ltoWidget);

        // custom args
        QWidget *customArgWidget = new QWidget(packCard);
        QHBoxLayout *customArgLayout = new QHBoxLayout(customArgWidget);
        customArgLayout->setContentsMargins(0, 0, 0, 0);
        ElaText *customArgText = new ElaText(tr("自定义命令参数"), 12, customArgWidget);
        ElaLineEdit *customArgEdit = new ElaLineEdit(customArgWidget);
        customArgLayout->addWidget(customArgText);
        customArgLayout->addWidget(customArgEdit);
        packLayout->addWidget(customArgWidget);

        scrollLayout->addWidget(packCard);

        // Connect signals and slots
        connect(standaloneButton, &QCheckBox::toggled, [=](bool checked) {
            PCM.set(PCE::Standalone, checked);
        });
        connect(onefileButton, &QCheckBox::toggled, [=](bool checked) {
            PCM.set(PCE::Onefile, checked);
        });
        connect(removePackFile, &QCheckBox::toggled, [=](bool checked) {
            PCM.set(PCE::RemoveOutput, checked);
        });
        connect(ltoAuto, &QCheckBox::toggled, [=](bool checked) {
            if (checked) PCM.set(PCE::LtoMode, QVariant::fromValue<LTOMode>(LTOMode::Auto));
        });
        connect(ltoYes, &QCheckBox::toggled, [=](bool checked) {
            if (checked) PCM.set(PCE::LtoMode, QVariant::fromValue<LTOMode>(LTOMode::Yes));
        });
        connect(ltoNo, &QCheckBox::toggled, [=](bool checked) {
            if (checked) PCM.set(PCE::LtoMode, QVariant::fromValue<LTOMode>(LTOMode::No));
        });
        connect(customArgEdit, &QLineEdit::textChanged, [=](const QString &arg) {
            PCM.set(PCE::CustomCommand, arg);
        });

        connect(&PCM, &ProjectConfigManager::updateUI, [=]() {
            standaloneButton->setChecked(PCM.getBool(PCE::Standalone));
            onefileButton->setChecked(PCM.getBool(PCE::Onefile));
            removePackFile->setChecked(PCM.getBool(PCE::RemoveOutput));
            switch (PCM.get(PCE::LtoMode).value<LTOMode>()) {
                case LTOMode::Auto:
                    ltoAuto->setChecked(true);
                    break;
                case LTOMode::Yes:
                    ltoYes->setChecked(true);
                    break;
                case LTOMode::No:
                    ltoNo->setChecked(true);
                    break;
            }
            customArgEdit->setText(PCM.getString(PCE::CustomCommand));
        });
        connect(ElaTheme::getInstance(), &ElaTheme::themeModeChanged, this, [=](ElaThemeType::ThemeMode mode) {
            const QColor &textColor = ElaThemeColor(mode, ThemeColor::BasicText);
            QString textColorHex = textColor.name();
            QString textStyleSheet = QString("color: %1;").arg(textColorHex);
            Utils::addWidgetStyleSheet(standaloneButton, textStyleSheet);
            Utils::addWidgetStyleSheet(onefileButton, textStyleSheet);
            Utils::addWidgetStyleSheet(removePackFile, textStyleSheet);
            Utils::addWidgetStyleSheet(ltoAuto, textStyleSheet);
            Utils::addWidgetStyleSheet(ltoYes, textStyleSheet);
            Utils::addWidgetStyleSheet(ltoNo, textStyleSheet);
        });
    }
#pragma endregion

    // Asset card
#pragma region AssetCardSection
    {
        this->assetCard = new ElaScrollPageArea(scrollWidget);
        assetCard->setFixedHeight(QWIDGETSIZE_MAX);
        assetCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *assetLayout = new QVBoxLayout(assetCard);
        assetLayout->setSpacing(18);
        assetLayout->setContentsMargins(20, 20, 20, 20);
        assetLayout->setAlignment(Qt::AlignTop);
        assetLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("资源设置"), 16, assetCard);
        assetLayout->addWidget(title);

        // asset
        QWidget *assetWidget = new QWidget(assetCard);
        QHBoxLayout *assetWidgetLayout = new QHBoxLayout(assetWidget);
        assetWidgetLayout->setContentsMargins(0, 0, 0, 0);
        ElaListView *assetList = new ElaListView(assetCard);
        assetList->setModel(this->assetListModel);
        QWidget *assetWidgetButtonGroup = new QWidget(assetWidget);
        QVBoxLayout *assetWidgetButtonGroupLayout = new QVBoxLayout(assetWidgetButtonGroup);
        assetWidgetButtonGroupLayout->setContentsMargins(10, 0, 0, 0);
        ElaPushButton *addFileButton = new ElaPushButton(tr("添加文件"), assetWidgetButtonGroup);
        Utils::setWidgetPixelSize(addFileButton, 12);
        ElaPushButton *addDirButton = new ElaPushButton(tr("添加目录"), assetWidgetButtonGroup);
        Utils::setWidgetPixelSize(addDirButton, 12);
        ElaPushButton *removeItemButton = new ElaPushButton(tr("删除项"), assetWidgetButtonGroup);
        Utils::setWidgetPixelSize(removeItemButton, 12);
        assetWidgetButtonGroupLayout->addWidget(addFileButton);
        assetWidgetButtonGroupLayout->addWidget(addDirButton);
        assetWidgetButtonGroupLayout->addWidget(removeItemButton);

        assetWidgetLayout->addWidget(assetList);
        assetWidgetLayout->addWidget(assetWidgetButtonGroup);
        assetLayout->addWidget(assetWidget);

        // icon
        QWidget *iconWidget = new QWidget(assetCard);
        QHBoxLayout *iconLayout = new QHBoxLayout(iconWidget);
        ElaText *iconText = new ElaText(tr("图标文件"), 12, iconWidget);
        ElaLineEdit *iconEdit = new ElaLineEdit(iconWidget);
        iconEdit->setToolTip(tr("如果图片文件格式不是.ico，那么环境中需要安装imageio库"));
        ElaPushButton *iconButton = new ElaPushButton(tr("浏览"), iconWidget);
        Utils::setWidgetPixelSize(iconButton, 12);
        iconLayout->addWidget(iconText);
        iconLayout->addWidget(iconEdit);
        iconLayout->addWidget(iconButton);
        assetLayout->addWidget(iconWidget);

        scrollLayout->addWidget(assetCard);

        // Connect signals and slots
        connect(iconEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::IconPath, text);
        });
        connect(iconButton, &QPushButton::clicked, [=]() {
            QString path = QFileDialog::getOpenFileName(this, "Nuitka Studio",
                                                        config.getString(ConfigItem::DefaultProjectPath),
                                                        "Images (*.ico *.png *.jpg *.jpeg);;All files(*)");
            iconEdit->setText(path);
            PCM.set(PCE::IconPath, path);
        });

        connect(addFileButton, &QPushButton::clicked, [=]() {
            QString filePath = QFileDialog::getOpenFileName(this, "Nuitka Studio",
                                                            config.getString(ConfigItem::DefaultDataPath));
            if (filePath.isEmpty()) {
                return;
            }
            QStringList dataList = this->assetListModel->stringList();
            dataList << filePath;
            this->assetListModel->setStringList(dataList);
            PCM.appendToStringList(PCE::DataList, filePath);
        });
        connect(addDirButton, &QPushButton::clicked, [=]() {
            QString dirPath = QFileDialog::getExistingDirectory(this, "Nuitka Studio  数据目录",
                                                                config.getString(ConfigItem::DefaultDataPath),
                                                                QFileDialog::ShowDirsOnly);
            if (dirPath.isEmpty()) {
                return;
            }

            QStringList dataList = this->assetListModel->stringList();
            dataList << dirPath;
            this->assetListModel->setStringList(dataList);
            PCM.appendToStringList(PCE::DataList, dirPath);
        });
        connect(removeItemButton, &QPushButton::clicked, [=]() {
            QModelIndex index = assetList->currentIndex();
            if (index.isValid() && this->assetListModel) {
                const QString &text = this->assetListModel->stringList().at(index.row());

                this->assetListModel->removeRow(index.row());
                PCM.removeFromStringList(PCE::DataList, text);
            }
        });

        connect(&PCM, &ProjectConfigManager::updateUI, [=]() {
            iconEdit->setText(PCM.getString(PCE::IconPath));
            this->assetListModel->setStringList(PCM.getStringList(PCE::DataList));
        });
    }
#pragma endregion

    // File info card
#pragma region FileInfoCardSection
    {
        this->fileInfoCard = new ElaScrollPageArea(scrollWidget);
        fileInfoCard->setFixedHeight(QWIDGETSIZE_MAX);
        fileInfoCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *fileInfoLayout = new QVBoxLayout(fileInfoCard);
        fileInfoLayout->setSpacing(18);
        fileInfoLayout->setContentsMargins(20, 20, 20, 20);
        fileInfoLayout->setAlignment(Qt::AlignTop);
        fileInfoLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("文件信息设置"), 16, fileInfoCard);
        fileInfoLayout->addWidget(title);

        // file version
        ElaLineEdit *fileVersionEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("文件版本号"), fileVersionEdit, nullptr, false);
        // company
        ElaLineEdit *companyEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("企业信息"), companyEdit, nullptr, false);
        // product name
        ElaLineEdit *productNameEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("产品名称"), productNameEdit, nullptr, false);
        // product version
        ElaLineEdit *productVersionEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("产品版本号"), productVersionEdit, nullptr, false);
        // copyright
        ElaLineEdit *copyrightEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("版权信息"), copyrightEdit, nullptr, false);
        // trademark
        ElaLineEdit *trademarkEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("商标信息"), trademarkEdit, nullptr, false);
        // file description
        ElaLineEdit *fileDescriptionEdit = new ElaLineEdit(fileInfoCard);
        addConfigRow(fileInfoLayout, fileInfoCard, tr("文件说明"), fileDescriptionEdit, nullptr, false);

        // Connect signals and slots
        connect(fileVersionEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::FileVersion, text);
        });
        connect(companyEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::Company, text);
        });
        connect(productNameEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::ProductName, text);
        });
        connect(productVersionEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::ProductVersion, text);
        });
        connect(copyrightEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::LegalCopyright, text);
        });
        connect(trademarkEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::LegalTrademarks, text);
        });
        connect(fileDescriptionEdit, &QLineEdit::textChanged, [=](const QString &text) {
            PCM.set(PCE::FileDescription, text);
        });

        connect(&PCM, &ProjectConfigManager::updateUI, [=]() {
            fileVersionEdit->setText(PCM.getString(PCE::FileVersion));
            companyEdit->setText(PCM.getString(PCE::Company));
            productNameEdit->setText(PCM.getString(PCE::ProductName));
            productVersionEdit->setText(PCM.getString(PCE::ProductVersion));
            copyrightEdit->setText(PCM.getString(PCE::LegalCopyright));
            trademarkEdit->setText(PCM.getString(PCE::LegalTrademarks));
            fileDescriptionEdit->setText(PCM.getString(PCE::FileDescription));
        });

        scrollLayout->addWidget(fileInfoCard);
    }
#pragma endregion

    // Console Card
#pragma region ConsoleCardSection
    {
        this->consoleCard = new ElaScrollPageArea(scrollWidget);
        consoleCard->setFixedHeight(QWIDGETSIZE_MAX);
        consoleCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        QVBoxLayout *consoleLayout = new QVBoxLayout(consoleCard);
        consoleLayout->setSpacing(18);
        consoleLayout->setContentsMargins(20, 20, 20, 20);
        consoleLayout->setAlignment(Qt::AlignTop);
        consoleLayout->setSizeConstraint(QLayout::SetMinimumSize);

        // title
        ElaText *title = new ElaText(tr("控制台"), 16, consoleCard);
        consoleLayout->addWidget(title);

        // console
        this->consoleEdit = new ElaPlainTextEdit(consoleCard);
        consoleLayout->addWidget(this->consoleEdit);

        // button group
        QWidget *buttonGroup = new QWidget(consoleCard);
        QHBoxLayout *buttonGroupLayout = new QHBoxLayout(buttonGroup);
        buttonGroupLayout->setContentsMargins(0, 0, 0, 0);

        // clear console
        ElaPushButton *clearConsoleButton = new ElaPushButton(tr("清空控制台"), buttonGroup);
        clearConsoleButton->setFixedWidth(100);
        Utils::setWidgetPixelSize(clearConsoleButton, 12);
        // start pack button
        this->startPackButton = new ElaPushButton(tr("开始打包"), buttonGroup);
        Utils::setWidgetPixelSize(this->startPackButton, 12);
        // stop pack button
        this->stopPackButton = new ElaPushButton(tr("停止打包"), buttonGroup);
        Utils::setWidgetPixelSize(this->stopPackButton, 12);
        this->stopPackButton->setEnabled(false);

        buttonGroupLayout->addWidget(clearConsoleButton);
        buttonGroupLayout->addStretch();
        buttonGroupLayout->addWidget(this->startPackButton);
        buttonGroupLayout->addWidget(this->stopPackButton);
        consoleLayout->addWidget(buttonGroup);

        scrollLayout->addWidget(consoleCard);

        connect(clearConsoleButton, &QPushButton::clicked, [=]() {
            this->consoleEdit->clear();
        });
        connect(this->startPackButton, &QPushButton::clicked, [=]() {
            emit this->startPack();
            this->packStart();
        });
        connect(this->stopPackButton, &QPushButton::clicked, [=]() {
            emit this->stopPack();
            this->packEnd();
        });
    }
#pragma endregion

    // scroll area config
    scrollLayout->addStretch();
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);

    // main layout
    mainLayout->addWidget(scrollArea);
}

void PackPage::addConsoleContent(const QString &content) const {
    this->consoleEdit->appendPlainText(content);
}

void PackPage::packStart() {
    this->isPacking = true;
    this->stopPackButton->setEnabled(true);
    this->startPackButton->setEnabled(false);
}

void PackPage::packEnd() {
    this->isPacking = false;
    stopPackButton->setEnabled(false);
    startPackButton->setEnabled(true);
}

void PackPage::scrollTo(const PageCard &card) const {
    switch (card) {
        case PageCard::PackPageBaseCard:
            this->scrollArea->ensureWidgetVisible(this->baseCard);
            break;
        case PageCard::PackPagePackCard:
            this->scrollArea->ensureWidgetVisible(this->packCard);
            break;
        case PageCard::PackPageAssetCard:
            this->scrollArea->ensureWidgetVisible(this->assetCard);
            break;
        case PageCard::PackPageFileInfoCard:
            this->scrollArea->ensureWidgetVisible(this->fileInfoCard);
            break;
        case PageCard::PackPageConsoleCard:
            this->scrollArea->ensureWidgetVisible(this->consoleCard);
            break;
        default:
            Logger::error("PackPage::scrollTo: invalid card");
            break;
    }
}
