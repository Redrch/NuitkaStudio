//
// Created by redrch on 2025/12/5.
//

#ifndef NUITKASTUDIO_CONFIG_H
#define NUITKASTUDIO_CONFIG_H

#include <QString>
#include <QVariant>
#include <QMap>
#include <QMetaEnum>
#include <QSize>
#include <QPoint>

#include <QDebug>

#include <QSettings>
#include <QDir>

#include "logger.h"
#include "utils.h"

namespace ConfigEnumNS {
    Q_NAMESPACE

    enum class EncodingEnum {
        utf8 = 0,
        gbk = 1,
        ascii = 2,
        NONE = -1
    };

    enum class ConfigItem {
        Language,
        ConsoleInputEncoding,
        ConsoleOutputEncoding,
        PackTimerTriggerInterval,
        MaxPackLogCount,
        IsShowCloseWindow,
        IsHideOnClose,
        NpfPath,
        IsLightMode,
        FloatButtonSize,
        FloatButtonOriginalSize,
        FloatButtonPos,
        IsFloatButtonLight,
        IsSplashScreen,
        DefaultPythonPath,
        DefaultMainFilePath,
        DefaultOutputPath,
        DefaultIconPath,
        DefaultDataPath,
        TempPath,
        NONE = -1
    };

    enum class ConfigGroup {
        General,
        DefaultPath
    };

    enum class Language {
        zh_CN,
        en_US,
        NONE = -1
    };

    Q_ENUM_NS(ConfigItem)
    Q_ENUM_NS(Language)
    Q_ENUM_NS(EncodingEnum)
    Q_ENUM_NS(ConfigGroup)
}

using namespace ConfigEnumNS;

using ConfigGroupMap = QMap<QString, QVariant>;
using ConfigMap = QMap<QString, ConfigGroupMap>;

Q_DECLARE_METATYPE(EncodingEnum)

// Singleton class
class Config {
    Q_GADGET

public:
    static Config &instance() {
        static Config inst;
        return inst;
    }

    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;

    Config();
    ~Config();

    void addConfig(const QString &key, const QVariant &defaultValue, const ConfigGroup &group) const;
    bool containsConfig(const QString &key) const;

    void writeConfig() const;
    void readConfig() const;

    // Getter and setter
    QVariant getConfig(const QString &configValue) const;
    QVariant getConfig(ConfigItem configValue) const;
    QString getConfigToString(ConfigItem configValue) const;
    int getConfigToInt(ConfigItem configValue) const;
    bool getConfigToBool(ConfigItem configValue) const;
    EncodingEnum getConfigToEncodingEnum(ConfigItem configValue) const;
    Language getConfigToLanguage(ConfigItem configValue) const;
    QSize getConfigToSize(ConfigItem configValue) const;
    QPoint getConfigToPoint(ConfigItem configValue) const;

    void setConfig(ConfigItem configValue, const QVariant &value) const;
    void setConfig(const QString &configValue, const QVariant &value) const;
    void setConfigFromString(ConfigItem configValue, const QString &string) const;
    void setConfigFromInt(ConfigItem configValue, int value) const;
    void setConfigFromBool(ConfigItem configValue, bool value) const;
    void setConfigFromEncodingEnum(ConfigItem configValue, EncodingEnum encodingValue) const;
    void setConfigFromLanguage(ConfigItem configValue, Language language) const;
    void setConfigFromSize(ConfigItem configValue, const QSize &size) const;
    void setConfigFromPoint(ConfigItem configValue, const QPoint &point) const;

    [[nodiscard]] const QString &getConfigPath() const {
        return this->configPath;
    }

    void setConfigPath(const QString &path) {
        this->configPath = path;
    };

private:
    QSettings *settings;
    QString configPath;
    ConfigMap *configMap;
};


#endif //NUITKASTUDIO_CONFIG_H
