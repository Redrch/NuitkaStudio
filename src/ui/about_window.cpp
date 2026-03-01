//
// Created by redrch on 2025/12/30.
//

// You may need to build the project (run Qt uic code generator) to get "ui_about_window.h" resolved

#include "about_window.h"

AboutWindow::AboutWindow(QWidget *parent) : ElaDialog(parent) {
    this->setWindowTitle("Nuitka Studio 关于");
    this->setFixedSize(550, 400);
    this->setWindowButtonFlags(ElaAppBarType::CloseButtonHint);

    QString textColor = ElaTheme::getInstance()->getThemeColor(ElaTheme::getInstance()->getThemeMode(),
                                                               ElaThemeType::BasicText).name();
    QString githubUrl = "https://github.com/Redrch/NuitkaStudio";

    ElaImageCard *logoCard = new ElaImageCard(this);
    logoCard->setFixedSize(60, 60);
    logoCard->setIsPreserveAspectCrop(false);
    logoCard->setCardImage(QImage(":/logo"));

    QVBoxLayout *logoCardLayout = new QVBoxLayout();
    logoCardLayout->addWidget(logoCard);
    logoCardLayout->addStretch();

    ElaText *nameText = new ElaText("Nuitka Studio", 18, this);
    QFont nameFont = nameText->font();
    nameFont.setWeight(QFont::Bold);
    nameText->setFont(nameFont);
    nameText->setWordWrap(false);

    ElaText *versionText = new ElaText(QString(APP_VERSION), 14, this);
    versionText->setWordWrap(false);
    ElaText *buildTimeText = new ElaText("构建时间: " + QString(APP_BUILD_TIME), 14, this);
    buildTimeText->setWordWrap(false);
    ElaText *licenseText = new ElaText("授权协议: Apache 2.0", 14, this);
    licenseText->setWordWrap(false);
    ElaText *emailText = new ElaText("作者邮箱: redrch327@gmail.com", 14, this);
    emailText->setWordWrap(false);
    emailText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    ElaText *githubUrlText = new ElaText(
        QString("Github仓库: <a href=\"%1\" style=\"color: %2;\">%3</a>")
        .arg(githubUrl).arg(textColor).arg(githubUrl),
        14, this);
    githubUrlText->setOpenExternalLinks(true);
    githubUrlText->setTextInteractionFlags(Qt::TextSelectableByMouse);
    githubUrlText->setCursor(Qt::PointingHandCursor);
    githubUrlText->setWordWrap(false);
    ElaText *copyrightText = new ElaText("版权所有 © 2026 Redrch", 14, this);
    copyrightText->setWordWrap(false);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(10);
    textLayout->addWidget(nameText);
    textLayout->addWidget(versionText);
    textLayout->addWidget(buildTimeText);
    textLayout->addWidget(licenseText);
    textLayout->addWidget(emailText);
    textLayout->addWidget(githubUrlText);
    textLayout->addSpacing(10);
    textLayout->addWidget(copyrightText);
    textLayout->addStretch();

    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->addSpacing(30);
    contentLayout->addLayout(logoCardLayout);
    contentLayout->addSpacing(30);
    contentLayout->addLayout(textLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 20, 0, 0);
    mainLayout->addLayout(contentLayout);
}

AboutWindow::~AboutWindow() = default;
