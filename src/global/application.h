//
// Created by redrch on 2026/4/15.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QSplashScreen>

#include "../ui/mainwindow.h"
#include "../utils/logger.h"
#include "../utils/config.h"

// Singleton
class Application : public QObject {
    Q_OBJECT

public:
    static Application& instance() {
        static Application instance;
        return instance;
    }
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    void init();
    void run();

public slots:
    void exit() const;
    void restart() const;
    void changeLanguage(const Language& language, bool isRestart = true) const;

private:
    Application();

    Logger* logger{};
    QSplashScreen* splash{};
    MainWindow* mainWindow{};
};

#define app Application::instance()

#endif //APPLICATION_H
