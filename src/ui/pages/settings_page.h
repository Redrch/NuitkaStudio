//
// Created by redrch on 2026/4/12.
//

#ifndef SETTINGS_PAGE_H
#define SETTINGS_PAGE_H

#include <ElaScrollArea.h>
#include <ElaScrollPageArea.h>
#include "types/data_structs.h"

class SettingsPage : public QWidget {
    Q_OBJECT

public:
    explicit SettingsPage(QWidget *parent = nullptr);

    void scrollTo(PageCard card) const;

protected:
    bool eventFilter(QObject *watch, QEvent *event) override;

private:
    ElaScrollArea *scrollArea;
    QWidget *generalCard;
    QWidget *appearanceCard;
    QWidget *defaultPathCard;

signals:
    Q_SIGNAL void opacityChanged(double value);
    Q_SIGNAL void blurChanged(int value);
    Q_SIGNAL void bgModeChanged();
};



#endif //SETTINGS_PAGE_H
