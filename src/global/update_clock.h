//
// Created by redrch on 2026/4/19.
//

#ifndef UPDATE_CLOCK_H
#define UPDATE_CLOCK_H

#include <QObject>
#include <QTimer>

// Singleton
class UpdateClock : public QObject {
    Q_OBJECT

public:
    static UpdateClock& instance() {
        static UpdateClock instance;
        return instance;
    }
    UpdateClock(const UpdateClock &) = delete;
    UpdateClock& operator=(const UpdateClock &) = delete;

    void start(int interval = 50);
    void end() const;

    [[nodiscard]] int getInterval() const;
    void setInterval(int interval);

private:
    UpdateClock();

    int interval{};
    QTimer* timer{};

signals:
    Q_SIGNAL void updateUI();
};

#endif //UPDATE_CLOCK_H
