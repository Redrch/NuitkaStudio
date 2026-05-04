//
// Created by redrch on 2025/12/5.
//

#include "config.h"

Config::Config() {
    this->configPath = "config.ini";
    this->configMap = new ConfigMap();
    this->settings = new QSettings(this->configPath, QSettings::IniFormat);
    this->settings->setIniCodec("UTF-8");
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
        const ConfigGroupMap& value = item.value();
        if (value.contains(key)) {
            return true;
        }
    }
    return false;
}

void Config::init() const {
    QSettings defaultSettings(":/default_config", QSettings::IniFormat);
    defaultSettings.setIniCodec("UTF-8");

    QStringList sections = defaultSettings.childGroups();
    for (const QString &section : sections) {
        defaultSettings.beginGroup(section);
        QStringList keys = defaultSettings.childKeys();
        for (const QString &key : keys) {
            this->addConfig(key, defaultSettings.value(key), Utils::stringToEnum<ConfigGroup>(section));
        }
        defaultSettings.endGroup();
    }
}

void Config::writeConfig() const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        const QString& groupMapName = item.key();
        ConfigGroupMap groupMap = item.value();
        this->settings->beginGroup(groupMapName);
        for (auto groupItem = groupMap.begin(); groupItem != groupMap.end(); ++groupItem) {
            this->settings->setValue(groupItem.key(), groupItem.value());
        }
        this->settings->endGroup();
    }
}

void Config::readConfig() const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        const QString& groupMapName = item.key();
        ConfigGroupMap value = item.value();
        this->settings->beginGroup(groupMapName);
        for (auto groupItem = value.begin(); groupItem != value.end(); ++groupItem) {
            const QString& groupKey = groupItem.key();
            this->set(groupKey, this->settings->value(groupKey), Utils::stringToEnum<ConfigGroup>(groupMapName));
        }
        this->settings->endGroup();
    }
}

QVariant Config::get(const QString &configValue, const ConfigGroup& group) const {
    if (group == ConfigGroup::NONE) {
        for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
            const ConfigGroupMap& value = item.value();
            if (value.contains(configValue)) {
                return value.value(configValue);
            }
        }
    }
    else {
        ConfigGroupMap groupMap = this->configMap->value(Utils::enumToString(group));
        if (groupMap.contains(configValue)) {
            return groupMap.value(configValue);
        }

    }
    Logger::error(QString("Config::getConfig: 值%1在config中不存在").arg(configValue));
    return {};
}

QVariant Config::get(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(Utils::enumToString(configValue), group);
}

QString Config::getString(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).toString();
}

int Config::getInt(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).toInt();
}

Encoding Config::getEncodingEnum(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).value<Encoding>();
}

Language Config::getLanguage(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).value<Language>();
}

bool Config::getBool(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).toBool();
}

QSize Config::getSize(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).toSize();
}

QPoint Config::getPoint(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).toPoint();
}

Theme Config::getTheme(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).value<Theme>();
}

WindowDisplayMode Config::getWindowDisplayMode(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).value<WindowDisplayMode>();
}

WindowBackground Config::getWindowBackground(const ConfigItem configValue, const ConfigGroup& group) const {
    return this->get(configValue, group).value<WindowBackground>();
}

void Config::set(const QString &configValue, const QVariant &value, const ConfigGroup& group) const {
    if (group == ConfigGroup::NONE) {
        bool found = false;
        for (auto it = this->configMap->begin(); it != this->configMap->end(); ++it) {
            if (it.value().contains(configValue)) {
                it.value().insert(configValue, value);
                found = true;
            }
        }
        if (!found) {
            Logger::error(QString("Config::set: 配置项 %1 在所有分组中都不存在").arg(configValue));
        }
    } else {
        QString groupStr = Utils::enumToString(group);
        if (this->configMap->contains(groupStr)) {
            ConfigGroupMap &groupMap = (*this->configMap)[groupStr];
            if (groupMap.contains(configValue)) {
                groupMap.insert(configValue, value);
            } else {
                Logger::error(QString("Config::set: 分组 %1 中不存在配置项 %2").arg(groupStr, configValue));
            }
        } else {
            Logger::error(QString("Config::set: 分组 %1 不存在").arg(groupStr));
        }
    }
}

void Config::set(const ConfigItem configValue, const QVariant &value, const ConfigGroup& group) const {
    return this->set(Utils::enumToString(configValue), value, group);
}

void Config::setString(const ConfigItem configValue, const QString &string, const ConfigGroup& group) const {
    this->set(configValue, QVariant(string), group);
}

void Config::setInt(const ConfigItem configValue, const int value, const ConfigGroup& group) const {
    this->set(configValue, QVariant(value), group);
}

void Config::setEncodingEnum(const ConfigItem configValue, const Encoding encodingValue, const ConfigGroup& group) const {
    this->set(configValue, QVariant::fromValue<Encoding>(encodingValue), group);
}

void Config::setLanguage(const ConfigItem configValue, Language language, const ConfigGroup& group) const {
    this->set(configValue, QVariant::fromValue<Language>(language), group);
}

void Config::setBool(const ConfigItem configValue, const bool value, const ConfigGroup& group) const {
    this->set(configValue, QVariant(value), group);
}

void Config::setSize(const ConfigItem configValue, const QSize &size, const ConfigGroup& group) const {
    this->set(configValue, QVariant(size), group);
}

void Config::setPoint(const ConfigItem configValue, const QPoint &point, const ConfigGroup& group) const {
    this->set(configValue, QVariant(point), group);
}

void Config::setTheme(ConfigItem configValue, const Theme &theme, const ConfigGroup &group) const {
    this->set(configValue, QVariant::fromValue<Theme>(theme), group);
}

void Config::setWindowDisplayMode(ConfigItem configValue, const WindowDisplayMode &displayMode, const ConfigGroup &group) const {
    this->set(configValue, QVariant::fromValue<WindowDisplayMode>(displayMode), group);
}

void Config::setWindowBackground(ConfigItem configValue, const WindowBackground &background, const ConfigGroup &group) const {
    this->set(configValue, QVariant::fromValue<WindowBackground>(background), group);
}

