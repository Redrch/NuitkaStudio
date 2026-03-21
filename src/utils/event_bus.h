//
// Created by redrch on 2026/3/8.
//

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <QObject>
#include "config.h"

class EventBus : public QObject {
    Q_OBJECT

public:
    static EventBus &instance() {
        static EventBus instance;
        return instance;
    }

    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;

    EventBus() = default;
    ~EventBus() override = default;

signals:
    Q_SIGNAL void languageChanged(Language language);
};


#endif //EVENT_BUS_H
