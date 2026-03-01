//
// Created by redrch on 2026/2/28.
//

#ifndef FLOAT_BUTTON_H
#define FLOAT_BUTTON_H

#include <ElaIconButton.h>
#include <QMouseEvent>
#include <QTimer>
#include <QApplication>
#include <QDebug>

#include "../utils/config.h"
#include "../types/simname.h"

struct PixmapGroup {
    QPixmap startLight;
    QPixmap startDark;
    QPixmap stopLight;
    QPixmap stopDark;
};

class FloatButton : public ElaIconButton {
    Q_OBJECT

public:
    explicit FloatButton(const PixmapGroup& pixmapGroup, QWidget *parent = nullptr);

    ~FloatButton() override;

    int getTriggerInterval() const;
    void setTriggerInterval(int interval);

    void packFinished();

signals:
    Q_SIGNAL void oneLongPressed();
    Q_SIGNAL void longPressed();
    Q_SIGNAL void startPack();
    Q_SIGNAL void stopPack();
    Q_SIGNAL void showMainWindow();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QTimer *longPressedTimer;
    QTimer *pressedTimer;

    bool isLongPressed = false;
    bool isPressed = false;
    int triggerInterval = 10;
    bool isPack = false;

    QPoint dragPos;
    PixmapGroup pixmapGroup;
};


#endif //FLOAT_BUTTON_H
