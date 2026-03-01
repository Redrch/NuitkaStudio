//
// Created by redrch on 2026/2/28.
//

#include "float_button.h"

FloatButton::FloatButton(const PixmapGroup& pixmapGroup, QWidget *parent): ElaIconButton(pixmapGroup.startLight, parent) {
    this->pixmapGroup = pixmapGroup;
    if (config.getConfigToBool(SettingsEnum::IsFloatButtonLight)) {
        this->setPixmap(pixmapGroup.startLight);
    } else {
        this->setPixmap(pixmapGroup.startDark);
    }
    this->move(config.getConfigToPoint(SettingsEnum::FloatButtonPos));
    this->setFixedSize(config.getConfigToSize(SettingsEnum::FloatButtonSize));

    this->longPressedTimer = new QTimer(this);
    this->pressedTimer = new QTimer(this);
    this->longPressedTimer->setSingleShot(false);

    connect(this->longPressedTimer, &QTimer::timeout, [=]() {
        if (!this->isLongPressed) {
            this->isLongPressed = true;
            this->longPressedTimer->setInterval(10);
        }
        emit oneLongPressed();
    });
    connect(this->pressedTimer, &QTimer::timeout, [=]() {
        if (this->isPressed) {
            emit longPressed();
        }
    });
    this->pressedTimer->setInterval(this->triggerInterval);
}

FloatButton::~FloatButton() = default;

int FloatButton::getTriggerInterval() const {
    return this->triggerInterval;
}

void FloatButton::setTriggerInterval(const int interval) {
    this->triggerInterval = interval;
    this->pressedTimer->setInterval(interval);
}

void FloatButton::packFinished() {
    this->isPack = false;
    if (config.getConfigToBool(SettingsEnum::IsFloatButtonLight)) {
        this->setPixmap(this->pixmapGroup.startLight);
    } else {
        this->setPixmap(this->pixmapGroup.startDark);
    }
    this->update();
}

void FloatButton::mousePressEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            this->dragPos = event->globalPos() - this->frameGeometry().topLeft();
            if (QApplication::keyboardModifiers() & Qt::AltModifier) {
                this->setFixedSize(config.getConfigToSize(SettingsEnum::FloatButtonOriginalSize));
            }
        }
        else if (QApplication::keyboardModifiers() & Qt::AltModifier) {
            if (config.getConfigToBool(SettingsEnum::IsFloatButtonLight)) {
                if (this->isPack) {
                    this->setPixmap(this->pixmapGroup.stopDark);
                } else {
                    this->setPixmap(this->pixmapGroup.startDark);
                }
                config.setConfigFromBool(SettingsEnum::IsFloatButtonLight, false);
            } else {
                if (this->isPack) {
                    this->setPixmap(this->pixmapGroup.stopLight);
                } else {
                    this->setPixmap(this->pixmapGroup.startLight);
                }
                config.setConfigFromBool(SettingsEnum::IsFloatButtonLight, true);
            }
        }
        else if (QApplication::keyboardModifiers() & Qt::ShiftModifier) {
            emit this->showMainWindow();
        }
        else {
            if (!this->isPack) {
                this->isPack = true;
                if (config.getConfigToBool(SettingsEnum::IsFloatButtonLight)) {
                    this->setPixmap(this->pixmapGroup.stopLight);
                } else {
                    this->setPixmap(this->pixmapGroup.stopDark);
                }
                emit this->startPack();
            }  else {
                this->isPack = false;
                if (config.getConfigToBool(SettingsEnum::IsFloatButtonLight)) {
                    this->setPixmap(this->pixmapGroup.startLight);
                } else {
                    this->setPixmap(this->pixmapGroup.startDark);
                }
                emit this->stopPack();
            }
        }
    }

    this->isLongPressed = false;
    this->isPressed = true;
    this->longPressedTimer->start(500);
    this->pressedTimer->start();

    ElaIconButton::mousePressEvent(event);
}

void FloatButton::mouseReleaseEvent(QMouseEvent *event) {
    this->longPressedTimer->stop();
    this->isPressed = false;
    this->pressedTimer->stop();

    if (!isLongPressed) {
        ElaIconButton::mouseReleaseEvent(event);
    } else {
        event->accept();
    }

    ElaIconButton::mouseReleaseEvent(event);
}

void FloatButton::mouseMoveEvent(QMouseEvent *event) {
    if (event->buttons() & Qt::LeftButton) {
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            this->move(event->globalPos() - this->dragPos);
            config.setConfigFromPoint(SettingsEnum::FloatButtonPos, event->globalPos() - this->dragPos);

            if (this->longPressedTimer->isActive()) {
                this->longPressedTimer->stop();
            }
            if (this->pressedTimer->isActive()) {
                this->pressedTimer->stop();
            }
        }
    }
    ElaIconButton::mouseMoveEvent(event);
}

void FloatButton::wheelEvent(QWheelEvent *event) {
    QPoint numSteps = event->angleDelta() / 120;

    if (!numSteps.isNull()) {
        int scrollY = numSteps.y();
        if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
            if (scrollY > 0) {
                QSize size = this->size();
                size *= 1.1;
                config.setConfigFromSize(SettingsEnum::FloatButtonSize, size);
                this->setFixedSize(size);
            } else if (scrollY < 0) {
                QSize size = this->size();
                size *= 0.9;
                config.setConfigFromSize(SettingsEnum::FloatButtonSize, size);
                this->setFixedSize(size);
            }
        }
    }

    ElaIconButton::wheelEvent(event);
}

