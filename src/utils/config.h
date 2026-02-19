//
// Created by redrch on 2025/12/5.
//

#ifndef NUITKASTUDIO_CONFIG_H
#define NUITKASTUDIO_CONFIG_H

#include <QString>
#include <QVariant>
#include <QMap>
#include <QMetaEnum>

#include <QDebug>

#include <QSettings>
#include <QDir>

#include "logger.h"

enum class EncodingEnum {
    utf8 = 0,
    gbk = 1,
    ascii = 2
};

namespace SettingsEnumNS {
    Q_NAMESPACE

    enum class SettingsEnum {
        DefaultPythonPath,
        DefaultMainFilePath,
        DefaultOutputPath,
        DefaultIconPath,
        DefaultDataPath,
        TempPath,
        ConsoleInputEncoding,
        ConsoleOutputEncoding,
        PackTimerTriggerInterval,
        MaxPackLogCount,
        IsShowCloseWindow,
        IsHideOnClose,
        NONE
    };
    Q_ENUM_NS(SettingsEnum)
}

using namespace SettingsEnumNS;

Q_DECLARE_METATYPE(EncodingEnum)

// Singleton class
class Config {
    Q_GADGET
public:
    static Config& instance() {
        static Config inst;
        return inst;
    }
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    Config();
    ~Config();

    void writeConfig();
    void readConfig();

    QString encodingEnumToString(EncodingEnum enumValue);
    int encodingEnumToInt(EncodingEnum enumValue);
    EncodingEnum encodingEnumFromString(const QString& string);
    EncodingEnum encodingEnumFromInt(int value);

    QString settingsEnumToString(SettingsEnum enumValue);
    SettingsEnum settingsEnumFromString(const QString& string);

    // Getter and setter
    QVariant getConfig(const QString &configValue) const;
    QVariant getConfig(SettingsEnum configValue);
    QString getConfigToString(SettingsEnum configValue);
    int getConfigToInt(SettingsEnum configValue);
    bool getConfigToBool(SettingsEnum configValue);
    EncodingEnum getConfigEncodingEnum(SettingsEnum configValue);

    void setConfig(SettingsEnum configValue, const QVariant& value);
    void setConfig(const QString &configValue, const QVariant& value) const;
    void setConfigFromString(SettingsEnum configValue, const QString& string);
    void setConfigFromInt(SettingsEnum configValue, int value);
    void setConfigFromBool(SettingsEnum configValue, bool value);
    void setConfigFromEncodingEnum(SettingsEnum configValue, EncodingEnum encodingValue);

    [[nodiscard]] const QString& getConfigPath() const {
        return this->configPath;
    }
    void setConfigPath(const QString& path) {
        this->configPath = path;
    };

private:
    QSettings* settings;
    QString configPath;
    QMap<QString, QVariant>* configMap;
};


#endif //NUITKASTUDIO_CONFIG_H
