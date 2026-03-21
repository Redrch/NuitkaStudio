//
// Created by redrch on 2026/2/15.
//

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <QMap>
#include <QVariant>
#include <QMutex>
#include <QTranslator>

// Singleton class
class GlobalData : public QObject {
    Q_OBJECT
public:
    static GlobalData &instance() {
        static GlobalData instance;
        return instance;
    }

    GlobalData(const GlobalData &) = delete;
    GlobalData &operator=(const GlobalData &) = delete;

    GlobalData();
    ~GlobalData() override;

    QVariant get(const QString &name) const;
    QString getString(const QString &name) const;
    bool getBool(const QString &name) const;
    int getInt(const QString &name) const;
    double getDouble(const QString &name) const;
    template <typename T>
    QList<T> getList(const QString &name) const {
        QMutexLocker lock(mutex);
        return this->dataMap->value(name).value<QList<T>>();
    }

    void set(const QString &name, const QVariant &value);
    void setString(const QString &name, const QString &value);
    void setBool(const QString &name, const bool &value);
    void setInt(const QString &name, const int &value);
    void setDouble(const QString &name, const double &value);
    void setStringList(const QString &name, const QStringList &value);
    template <typename T>
    void setList(const QString &name, const QList<T> &value) {
        QMutexLocker locker(mutex);
        this->dataMap->insert(name, QVariant::fromValue(value));
        emit this->valueChanged(name, QVariant::fromValue(value));
    }

    void addToStringList(const QString &name, const QString &value) const;
    template <typename T>
    void addToList(const QString &name, const T &value) {
        QMutexLocker locker(mutex);
        QList<T> list = this->dataMap->value(name).value<QList<T>>();
        list.append(value);
        this->dataMap->insert(name, list);
    }

private:
    QMap<QString, QVariant>* dataMap;
    mutable QMutex* mutex;

signals:
    Q_SIGNAL void valueChanged(const QString& valueName, const QVariant& newValue);
};


namespace GlobalDataItemName {
    inline const QString &npfFilePath = "npf_file_path";
    inline const QString &isOpenNPF = "is_open_npf";
    inline const QString &translator = "translator";
    inline const QString &packLogPath = "pack_log_path";
}

#endif //GLOBAL_DATA_H
