//
// Created by redrch on 2025/12/30.
//

#ifndef NUITKASTUDIO_ABOUT_WINDOW_H
#define NUITKASTUDIO_ABOUT_WINDOW_H

#include <QWidget>
#include <QFile>
#include <QTextEdit>

#include "app_config.h"


QT_BEGIN_NAMESPACE
namespace Ui { class AboutWindow; }
QT_END_NAMESPACE

class AboutWindow : public QWidget {
Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);

    ~AboutWindow() override;

private:
    Ui::AboutWindow *ui;

private slots:
    void onLicensesButtonClicked();
};


#endif //NUITKASTUDIO_ABOUT_WINDOW_H
