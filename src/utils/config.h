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
#include <QDataStream>

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
        IsSavePackLog,

        BasicSettings,
        PackAndData,
        FileInfo,
        Console,

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
        Window,
        DefaultPath
    };

    enum class Language {
        zh_CN,
        en_US,
        NONE = -1
    };

    // stream
    inline QDataStream &operator<<(QDataStream &out, const Language &lang) {
        out << static_cast<int>(lang);
        return out;
    }

    inline QDataStream &operator>>(QDataStream &in, Language &lang) {
        int val;
        in >> val;
        lang = static_cast<Language>(val);
        return in;
    }

    Q_ENUM_NS(ConfigItem)
    Q_ENUM_NS(Language)
    Q_ENUM_NS(EncodingEnum)
    Q_ENUM_NS(ConfigGroup)
}

using namespace ConfigEnumNS;

using ConfigGroupMap = QMap<QString, QVariant>;
using ConfigMap = QMap<QString, ConfigGroupMap>;

Q_DECLARE_METATYPE(Language)
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
    QVariant get(const QString &configValue) const;
    QVariant get(ConfigItem configValue) const;
    QString getString(ConfigItem configValue) const;
    int getInt(ConfigItem configValue) const;
    bool getBool(ConfigItem configValue) const;
    EncodingEnum getEncodingEnum(ConfigItem configValue) const;
    Language getLanguage(ConfigItem configValue) const;
    QSize getSize(ConfigItem configValue) const;
    QPoint getPoint(ConfigItem configValue) const;

    void set(ConfigItem configValue, const QVariant &value) const;
    void set(const QString &configValue, const QVariant &value) const;
    void setString(ConfigItem configValue, const QString &string) const;
    void setInt(ConfigItem configValue, int value) const;
    void setBool(ConfigItem configValue, bool value) const;
    void setEncodingEnum(ConfigItem configValue, EncodingEnum encodingValue) const;
    void setLanguage(ConfigItem configValue, Language language) const;
    void setSize(ConfigItem configValue, const QSize &size) const;
    void setPoint(ConfigItem configValue, const QPoint &point) const;

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
