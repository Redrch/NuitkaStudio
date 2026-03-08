//
// Created by redrch on 2025/12/5.
//

#include "config.h"

Config::Config() {
    this->configPath = "config.ini";
    this->configMap = new ConfigMap();
    this->settings = new QSettings(this->configPath, QSettings::IniFormat);

    this->addConfig("Language", QVariant::fromValue<Language>(Language::zh_CN), ConfigGroup::General);
    this->addConfig("ConsoleInputEncoding", QVariant::fromValue<EncodingEnum>(EncodingEnum::utf8),
                    ConfigGroup::General);
    this->addConfig("ConsoleOutputEncoding", QVariant::fromValue<EncodingEnum>(EncodingEnum::utf8),
                    ConfigGroup::General);
    this->addConfig("PackTimerTriggerInterval", 100, ConfigGroup::General);
    this->addConfig("MaxPackLogCount", 20, ConfigGroup::General);
    this->addConfig("IsShowCloseWindow", QVariant(true), ConfigGroup::General);
    this->addConfig("IsHideOnClose", QVariant(true), ConfigGroup::General);
    this->addConfig("TempPath", QDir::tempPath() + "/NuitkaStudio", ConfigGroup::General);
    this->addConfig("NpfPath", "", ConfigGroup::General);
    this->addConfig("IsLightMode", QVariant(true), ConfigGroup::General);
    this->addConfig("FloatButtonSize", QVariant(QSize(25, 25)), ConfigGroup::General);
    this->addConfig("FloatButtonOriginalSize", QSize(25, 25), ConfigGroup::General);
    this->addConfig("FloatButtonPos", QVariant(QPoint(0, 0)), ConfigGroup::General);
    this->addConfig("IsFloatButtonLight", QVariant(true), ConfigGroup::General);
    this->addConfig("IsSplashScreen", QVariant(true), ConfigGroup::General);

    this->addConfig("DefaultPythonPath", "C:/", ConfigGroup::DefaultPath);
    this->addConfig("DefaultMainFilePath", "C:/", ConfigGroup::DefaultPath);
    this->addConfig("DefaultOutputPath", "C:/", ConfigGroup::DefaultPath);
    this->addConfig("DefaultIconPath", "C:/", ConfigGroup::DefaultPath);
    this->addConfig("DefaultDataPath", "C:/", ConfigGroup::DefaultPath);
}

Config::~Config() {
    delete this->settings;
}

void Config::addConfig(const QString &key, const QVariant &defaultValue, const ConfigGroup &group) const {
    if (this->containsConfig(key)) {
        Logger::error(QString("配置项%1已存在，不需要再次添加").arg(key));
        return;
    }
    ConfigGroupMap gmap = this->configMap->value(Utils::enumToString(group));
    gmap.insert(key, defaultValue);
    this->configMap->insert(Utils::enumToString(group), gmap);
}

bool Config::containsConfig(const QString &key) const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        ConfigGroupMap value = item.value();
        if (value.contains(key)) {
            return true;
        }
    }
    return false;
}

void Config::writeConfig() const {
    this->settings->beginGroup("General");
    // this->settings->setValue("Language", QVariant::fromValue<Language>(this))
    this->settings->setValue("ConsoleInputEncoding",
                             Utils::enumToInt<EncodingEnum>(
                                 this->getConfigToEncodingEnum(ConfigItem::ConsoleInputEncoding)));
    this->settings->setValue("ConsoleOutputEncoding",
                             Utils::enumToInt<EncodingEnum>(
                                 this->getConfigToEncodingEnum(ConfigItem::ConsoleOutputEncoding)));
    this->settings->setValue("PackTimerTriggerInterval", this->getConfig(ConfigItem::PackTimerTriggerInterval));
    this->settings->setValue("MaxPackLogCount", this->getConfig(ConfigItem::MaxPackLogCount));
    this->settings->setValue("IsShowCloseWindow", this->getConfig(ConfigItem::IsShowCloseWindow));
    this->settings->setValue("IsHideOnClose", this->getConfig(ConfigItem::IsHideOnClose));
    this->settings->setValue("TempPath", this->getConfig(ConfigItem::TempPath));
    this->settings->setValue("NpfPath", this->getConfig(ConfigItem::NpfPath));
    this->settings->setValue("IsLightMode", this->getConfig(ConfigItem::IsLightMode));
    this->settings->setValue("FloatButtonSize", this->getConfig(ConfigItem::FloatButtonSize));
    this->settings->setValue("FloatButtonOriginalSize", this->getConfig(ConfigItem::FloatButtonOriginalSize));
    this->settings->setValue("FloatButtonPos", this->getConfig(ConfigItem::FloatButtonPos));
    this->settings->setValue("IsFloatButtonLight", this->getConfig(ConfigItem::IsFloatButtonLight));
    this->settings->setValue("IsSplashScreen", this->getConfig(ConfigItem::IsSplashScreen));
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->settings->setValue("DefaultPythonPath", this->getConfig(ConfigItem::DefaultPythonPath));
    this->settings->setValue("DefaultMainFilePath", this->getConfig(ConfigItem::DefaultMainFilePath));
    this->settings->setValue("DefaultOutputPath", this->getConfig(ConfigItem::DefaultOutputPath));
    this->settings->setValue("DefaultIconPath", this->getConfig(ConfigItem::DefaultIconPath));
    this->settings->setValue("DefaultDataPath", this->getConfig(ConfigItem::DefaultDataPath));
    this->settings->endGroup();
}

void Config::readConfig() const {
    this->settings->beginGroup("General");
    this->setConfigFromEncodingEnum(ConfigItem::ConsoleInputEncoding,
                                    Utils::intToEnum<EncodingEnum>(
                                        this->settings->value("ConsoleInputEncoding").toInt()));
    this->setConfigFromEncodingEnum(ConfigItem::ConsoleOutputEncoding,
                                    Utils::intToEnum<EncodingEnum>(
                                        this->settings->value("ConsoleOutputEncoding").toInt())
    );
    this->setConfig(ConfigItem::PackTimerTriggerInterval, this->settings->value("PackTimerTriggerInterval").toInt());
    this->setConfig(ConfigItem::MaxPackLogCount, this->settings->value("MaxPackLogCount").toInt());
    this->setConfig(ConfigItem::IsShowCloseWindow, this->settings->value("IsShowCloseWindow").toBool());
    this->setConfig(ConfigItem::IsHideOnClose, this->settings->value("IsHideOnClose").toBool());
    this->setConfig(ConfigItem::TempPath, this->settings->value("TempPath").toString());
    this->setConfig(ConfigItem::NpfPath, this->settings->value("NpfPath").toString());
    this->setConfig(ConfigItem::IsLightMode, this->settings->value("IsLightMode").toBool());
    this->setConfig(ConfigItem::FloatButtonSize, this->settings->value("FloatButtonSize").toSize());
    this->setConfig(ConfigItem::FloatButtonOriginalSize, this->settings->value("FloatButtonOriginalSize").toSize());
    this->setConfig(ConfigItem::FloatButtonPos, this->settings->value("FloatButtonPos").toPoint());
    this->setConfig(ConfigItem::IsFloatButtonLight, this->settings->value("IsFloatButtonLight").toBool());
    this->setConfig(ConfigItem::IsSplashScreen, this->settings->value("IsSplashScreen").toBool());
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->setConfig(ConfigItem::DefaultPythonPath, this->settings->value("DefaultPythonPath").toString());
    this->setConfig(ConfigItem::DefaultMainFilePath, this->settings->value("DefaultMainFilePath").toString());
    this->setConfig(ConfigItem::DefaultOutputPath, this->settings->value("DefaultOutputPath").toString());
    this->setConfig(ConfigItem::DefaultIconPath, this->settings->value("DefaultIconPath").toString());
    this->setConfig(ConfigItem::DefaultDataPath, this->settings->value("DefaultDataPath").toString());
    this->settings->endGroup();
}

QVariant Config::getConfig(const QString &configValue) const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        ConfigGroupMap value = item.value();
        if (value.contains(configValue)) {
            return value.value(configValue);
        }
    }
    Logger::error(QString("Config::getConfig: 值%1在config中不存在").arg(configValue));
    return QVariant();
}

QVariant Config::getConfig(const ConfigItem configValue) const {
    return this->getConfig(Utils::enumToString(configValue));
}

QString Config::getConfigToString(const ConfigItem configValue) const {
    return this->getConfig(configValue).toString();
}

int Config::getConfigToInt(const ConfigItem configValue) const {
    return this->getConfig(configValue).toInt();
}

EncodingEnum Config::getConfigToEncodingEnum(const ConfigItem configValue) const {
    return this->getConfig(configValue).value<EncodingEnum>();
}

Language Config::getConfigToLanguage(const ConfigItem configValue) const {
    return this->getConfig(configValue).value<Language>();
}

bool Config::getConfigToBool(const ConfigItem configValue) const {
    return this->getConfig(configValue).toBool();
}

QSize Config::getConfigToSize(const ConfigItem configValue) const {
    return this->getConfig(configValue).toSize();
}

QPoint Config::getConfigToPoint(const ConfigItem configValue) const {
    return this->getConfig(configValue).toPoint();
}

void Config::setConfig(const QString &configValue, const QVariant &value) const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        ConfigGroupMap &v = item.value();
        if (v.contains(configValue)) {
            v.insert(configValue, value);
        }
    }
}

void Config::setConfig(const ConfigItem configValue, const QVariant &value) const {
    return this->setConfig(Utils::enumToString(configValue), value);
}

void Config::setConfigFromString(const ConfigItem configValue, const QString &string) const {
    this->setConfig(configValue, QVariant(string));
}

void Config::setConfigFromInt(const ConfigItem configValue, const int value) const {
    this->setConfig(configValue, QVariant(value));
}

void Config::setConfigFromEncodingEnum(const ConfigItem configValue, const EncodingEnum encodingValue) const {
    this->setConfig(configValue, QVariant::fromValue<EncodingEnum>(encodingValue));
}

void Config::setConfigFromLanguage(const ConfigItem configValue, Language language) const {
    this->setConfig(configValue, QVariant::fromValue<Language>(language));
}

void Config::setConfigFromBool(const ConfigItem configValue, const bool value) const {
    this->setConfig(configValue, QVariant(value));
}

void Config::setConfigFromSize(const ConfigItem configValue, const QSize &size) const {
    this->setConfig(configValue, QVariant(size));
}

void Config::setConfigFromPoint(const ConfigItem configValue, const QPoint &point) const {
    this->setConfig(configValue, QVariant(point));
}
