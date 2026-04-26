//
// Created by redrch on 2026/4/12.
//

#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <QWidget>

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

protected:
    bool eventFilter(QObject *watch, QEvent *event) override;
};



#endif //SETTINGS_PAGE_H
