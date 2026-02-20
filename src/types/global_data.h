//
// Created by redrch on 2026/2/15.
//

#ifndef GLOBAL_DATA_H
#define GLOBAL_DATA_H

#include <QMap>
#include <QVariant>
#include <QMutex>

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
    ~GlobalData();

    QVariant get(const QString &name) const;
    QString getString(const QString &name) const;
    bool getBool(const QString &name) const;
    int getInt(const QString &name) const;
    double getDouble(const QString &name) const;

    void set(const QString &name, const QVariant &value);
    void setString(const QString &name, const QString &value);
    void setBool(const QString &name, const bool &value);
    void setInt(const QString &name, const int &value);
    void setDouble(const QString &name, const double &value);

private:
    QMap<QString, QVariant>* dataMap;
    mutable QMutex* mutex;

signals:
    void valueChanged(const QString& valueName, const QVariant& newValue);
};

namespace GlobalDataItemName {
    inline const QString &NPF_FILE_PATH = "npf_file_path";
    inline const QString &IS_OPEN_NPF = "is_open_npf";
}

#endif //GLOBAL_DATA_H
