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

#include "../utils/logger.h"
#include "../utils/utils.h"

namespace ConfigEnumNS {
    Q_NAMESPACE

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

        DefaultPythonPath,
        DefaultProjectPath,
        DefaultDataPath,
        TempPath,

        Theme,
        WindowDisplayMode,
        WindowBackground,
        PixmapPath,
        MoviePath,
        NONE = -1
    };

    enum class ConfigGroup {
        General,
        Appearance,
        DefaultPath,
        NONE = -1
    };

    enum class Encoding {
        utf8,
        gbk,
        ascii,
        NONE = -1
    };

    enum class Language {
        zh_CN,
        en_US,
        NONE = -1
    };

    enum class Theme {
        Light,
        Dark,
        NONE = -1
    };

    enum class WindowDisplayMode {
        Normal,
        ElaMica,
        Mica,
        MicaAlt,
        Acrylic,
        DwmBlurnormal,
        NONE = -1
    };

    enum class WindowBackground {
        Normal,
        Pixmap,
        Movie,
        NONE = -1
    };

    // stream
    // Language
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

    // Encoding
    inline QDataStream &operator<<(QDataStream &out, const Encoding &encoding) {
        out << static_cast<int>(encoding);
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, Encoding &encoding) {
        int val;
        in >> val;
        encoding = static_cast<Encoding>(val);
        return in;
    }

    // Theme
    inline QDataStream &operator<<(QDataStream &out, const Theme &theme) {
        out << static_cast<int>(theme);
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, Theme &theme) {
        int val;
        in >> val;
        theme = static_cast<Theme>(val);
        return in;
    }

    // Window Display Mode
    inline QDataStream &operator<<(QDataStream &out, const WindowDisplayMode &displayMode) {
        out << static_cast<int>(displayMode);
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, WindowDisplayMode &displayMode) {
        int val;
        in >> val;
        displayMode = static_cast<WindowDisplayMode>(val);
        return in;
    }

    // window background
    inline QDataStream &operator<<(QDataStream &out, const WindowBackground &encoding) {
        out << static_cast<int>(encoding);
        return out;
    }
    inline QDataStream &operator>>(QDataStream &in, WindowBackground &encoding) {
        int val;
        in >> val;
        encoding = static_cast<WindowBackground>(val);
        return in;
    }

    Q_ENUM_NS(ConfigItem)
    Q_ENUM_NS(Language)
    Q_ENUM_NS(Encoding)
    Q_ENUM_NS(ConfigGroup)
    Q_ENUM_NS(Theme)
    Q_ENUM_NS(WindowDisplayMode)
    Q_ENUM_NS(WindowBackground)
}

using namespace ConfigEnumNS;

using ConfigGroupMap = QMap<QString, QVariant>;
using ConfigMap = QMap<QString, ConfigGroupMap>;

Q_DECLARE_METATYPE(Language)
Q_DECLARE_METATYPE(Encoding)
Q_DECLARE_METATYPE(Theme)
Q_DECLARE_METATYPE(WindowDisplayMode)
Q_DECLARE_METATYPE(WindowBackground)

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

    void init() const;
    void writeConfig() const;
    void readConfig() const;

    // Getter and setter
    QVariant get(const QString &configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    QVariant get(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    QString getString(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    int getInt(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    bool getBool(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    Encoding getEncodingEnum(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    Language getLanguage(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    QSize getSize(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    QPoint getPoint(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    Theme getTheme(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    WindowDisplayMode getWindowDisplayMode(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    WindowBackground getWindowBackground(ConfigItem configValue, const ConfigGroup& group = ConfigGroup::NONE) const;

    void set(ConfigItem configValue, const QVariant &value, const ConfigGroup& group = ConfigGroup::NONE) const;
    void set(const QString &configValue, const QVariant &value, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setString(ConfigItem configValue, const QString &string, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setInt(ConfigItem configValue, int value, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setBool(ConfigItem configValue, bool value, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setEncodingEnum(ConfigItem configValue, Encoding encodingValue, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setLanguage(ConfigItem configValue, Language language, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setSize(ConfigItem configValue, const QSize &size, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setPoint(ConfigItem configValue, const QPoint &point, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setTheme(ConfigItem configValue, const Theme &theme, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setWindowDisplayMode(ConfigItem configValue, const WindowDisplayMode &displayMode, const ConfigGroup& group = ConfigGroup::NONE) const;
    void setWindowBackground(ConfigItem configValue, const WindowBackground &background, const ConfigGroup& group) const;

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
