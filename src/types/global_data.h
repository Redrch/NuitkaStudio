//
// Created by redrch on 2026/2/15.
//

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <QMap>
#include <QVariant>
#include <QMutex>

// Singleton class
class GlobalData {
public:
    static GlobalData &instance() {
        static GlobalData instance;
        return instance;
    }

    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;

    GlobalData();
    ~GlobalData();

    QVariant get(const QString &name) const;
    void set(const QString &name, const QVariant &value) const;

private:
    QMap<QString, QVariant>* dataMap;
    mutable QMutex* mutex;
};


#endif //GLOBAL_DATA_H
