//
// Created by redrch on 2026/4/19.
//

#include "update_clock.h"
#include "utils/logger.h"
#include "project_config_manager.h"
#include "types/simname.h"

UpdateClock::UpdateClock() = default;

void UpdateClock::start(int interval) {
    this->interval = interval;
    if (!this->timer) {
        this->timer = new QTimer(this);
    }
    this->timer->setInterval(interval);
    connect(this->timer, &QTimer::timeout, this, [this] {
        emit this->updateUI();
        emit PCM.updateUI();
    });
    this->timer->start();
}

void UpdateClock::end() const {
    this->timer->stop();
}
int UpdateClock::getInterval() const {
    return this->interval;
}

void UpdateClock::setInterval(const int interval) {
    if (this->interval > 1000) {
        Logger::error("UpdateClock::setInterval: ui时钟刷新间隔过大，请将刷新间隔设置在1000ms及以下");
        return;
    }
    this->interval = interval;
    this->timer->setInterval(interval);
}

