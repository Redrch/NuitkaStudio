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
    this->addConfig("IsSavePackLog", QVariant(false), ConfigGroup::General);

    this->addConfig("BasicSettings", QVariant(true), ConfigGroup::Window);
    this->addConfig("PackAndData", QVariant(true), ConfigGroup::Window);
    this->addConfig("FileInfo", QVariant(true), ConfigGroup::Window);
    this->addConfig("Console", QVariant(true), ConfigGroup::Window);

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
    this->settings->setValue("Language", QVariant::fromValue<Language>(
                                 this->getLanguage(ConfigItem::Language)));
    this->settings->setValue("ConsoleInputEncoding",
                             Utils::enumToInt<EncodingEnum>(
                                 this->getEncodingEnum(ConfigItem::ConsoleInputEncoding)));
    this->settings->setValue("ConsoleOutputEncoding",
                             Utils::enumToInt<EncodingEnum>(
                                 this->getEncodingEnum(ConfigItem::ConsoleOutputEncoding)));
    this->settings->setValue("PackTimerTriggerInterval", this->get(ConfigItem::PackTimerTriggerInterval));
    this->settings->setValue("MaxPackLogCount", this->get(ConfigItem::MaxPackLogCount));
    this->settings->setValue("IsShowCloseWindow", this->get(ConfigItem::IsShowCloseWindow));
    this->settings->setValue("IsHideOnClose", this->get(ConfigItem::IsHideOnClose));
    this->settings->setValue("TempPath", this->get(ConfigItem::TempPath));
    this->settings->setValue("NpfPath", this->get(ConfigItem::NpfPath));
    this->settings->setValue("IsLightMode", this->get(ConfigItem::IsLightMode));
    this->settings->setValue("FloatButtonSize", this->get(ConfigItem::FloatButtonSize));
    this->settings->setValue("FloatButtonOriginalSize", this->get(ConfigItem::FloatButtonOriginalSize));
    this->settings->setValue("FloatButtonPos", this->get(ConfigItem::FloatButtonPos));
    this->settings->setValue("IsFloatButtonLight", this->get(ConfigItem::IsFloatButtonLight));
    this->settings->setValue("IsSplashScreen", this->get(ConfigItem::IsSplashScreen));
    this->settings->setValue("IsSavePackLog", this->get(ConfigItem::IsSavePackLog));
    this->settings->endGroup();

    this->settings->beginGroup("Window");
    this->settings->setValue("BasicSettings", this->get(ConfigItem::Console));
    this->settings->setValue("PackAndData", this->get(ConfigItem::Console));
    this->settings->setValue("FileInfo", this->get(ConfigItem::Console));
    this->settings->setValue("Console", this->get(ConfigItem::Console));
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->settings->setValue("DefaultPythonPath", this->get(ConfigItem::DefaultPythonPath));
    this->settings->setValue("DefaultMainFilePath", this->get(ConfigItem::DefaultMainFilePath));
    this->settings->setValue("DefaultOutputPath", this->get(ConfigItem::DefaultOutputPath));
    this->settings->setValue("DefaultIconPath", this->get(ConfigItem::DefaultIconPath));
    this->settings->setValue("DefaultDataPath", this->get(ConfigItem::DefaultDataPath));
    this->settings->endGroup();
}

void Config::readConfig() const {
    this->settings->beginGroup("General");
    this->set(ConfigItem::Language, this->settings->value("Language"));
    this->setEncodingEnum(ConfigItem::ConsoleInputEncoding,
                          Utils::intToEnum<EncodingEnum>(
                              this->settings->value("ConsoleInputEncoding").toInt()));
    this->setEncodingEnum(ConfigItem::ConsoleOutputEncoding,
                          Utils::intToEnum<EncodingEnum>(
                              this->settings->value("ConsoleOutputEncoding").toInt())
    );
    this->set(ConfigItem::PackTimerTriggerInterval, this->settings->value("PackTimerTriggerInterval").toInt());
    this->set(ConfigItem::MaxPackLogCount, this->settings->value("MaxPackLogCount").toInt());
    this->set(ConfigItem::IsShowCloseWindow, this->settings->value("IsShowCloseWindow").toBool());
    this->set(ConfigItem::IsHideOnClose, this->settings->value("IsHideOnClose").toBool());
    this->set(ConfigItem::TempPath, this->settings->value("TempPath").toString());
    this->set(ConfigItem::NpfPath, this->settings->value("NpfPath").toString());
    this->set(ConfigItem::IsLightMode, this->settings->value("IsLightMode").toBool());
    this->set(ConfigItem::FloatButtonSize, this->settings->value("FloatButtonSize").toSize());
    this->set(ConfigItem::FloatButtonOriginalSize, this->settings->value("FloatButtonOriginalSize").toSize());
    this->set(ConfigItem::FloatButtonPos, this->settings->value("FloatButtonPos").toPoint());
    this->set(ConfigItem::IsFloatButtonLight, this->settings->value("IsFloatButtonLight").toBool());
    this->set(ConfigItem::IsSplashScreen, this->settings->value("IsSplashScreen").toBool());
    this->set(ConfigItem::IsSavePackLog, this->settings->value("IsSavePackLog").toBool());
    this->settings->endGroup();

    this->settings->beginGroup("Window");
    this->set(ConfigItem::BasicSettings, this->settings->value("BasicSettings").toString());
    this->set(ConfigItem::PackAndData, this->settings->value("PackAndData").toString());
    this->set(ConfigItem::FileInfo, this->settings->value("FileInfo").toString());
    this->set(ConfigItem::Console, this->settings->value("Console").toString());
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->set(ConfigItem::DefaultPythonPath, this->settings->value("DefaultPythonPath").toString());
    this->set(ConfigItem::DefaultMainFilePath, this->settings->value("DefaultMainFilePath").toString());
    this->set(ConfigItem::DefaultOutputPath, this->settings->value("DefaultOutputPath").toString());
    this->set(ConfigItem::DefaultIconPath, this->settings->value("DefaultIconPath").toString());
    this->set(ConfigItem::DefaultDataPath, this->settings->value("DefaultDataPath").toString());
    this->settings->endGroup();
}

QVariant Config::get(const QString &configValue) const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        ConfigGroupMap value = item.value();
        if (value.contains(configValue)) {
            return value.value(configValue);
        }
    }
    Logger::error(QString("Config::getConfig: 值%1在config中不存在").arg(configValue));
    return QVariant();
}

QVariant Config::get(const ConfigItem configValue) const {
    return this->get(Utils::enumToString(configValue));
}

QString Config::getString(const ConfigItem configValue) const {
    return this->get(configValue).toString();
}

int Config::getInt(const ConfigItem configValue) const {
    return this->get(configValue).toInt();
}

EncodingEnum Config::getEncodingEnum(const ConfigItem configValue) const {
    return this->get(configValue).value<EncodingEnum>();
}

Language Config::getLanguage(const ConfigItem configValue) const {
    return this->get(configValue).value<Language>();
}

bool Config::getBool(const ConfigItem configValue) const {
    return this->get(configValue).toBool();
}

QSize Config::getSize(const ConfigItem configValue) const {
    return this->get(configValue).toSize();
}

QPoint Config::getPoint(const ConfigItem configValue) const {
    return this->get(configValue).toPoint();
}

void Config::set(const QString &configValue, const QVariant &value) const {
    for (auto item = this->configMap->begin(); item != this->configMap->end(); ++item) {
        ConfigGroupMap &v = item.value();
        if (v.contains(configValue)) {
            v.insert(configValue, value);
        }
    }
}

void Config::set(const ConfigItem configValue, const QVariant &value) const {
    return this->set(Utils::enumToString(configValue), value);
}

void Config::setString(const ConfigItem configValue, const QString &string) const {
    this->set(configValue, QVariant(string));
}

void Config::setInt(const ConfigItem configValue, const int value) const {
    this->set(configValue, QVariant(value));
}

void Config::setEncodingEnum(const ConfigItem configValue, const EncodingEnum encodingValue) const {
    this->set(configValue, QVariant::fromValue<EncodingEnum>(encodingValue));
}

void Config::setLanguage(const ConfigItem configValue, Language language) const {
    this->set(configValue, QVariant::fromValue<Language>(language));
}

void Config::setBool(const ConfigItem configValue, const bool value) const {
    this->set(configValue, QVariant(value));
}

void Config::setSize(const ConfigItem configValue, const QSize &size) const {
    this->set(configValue, QVariant(size));
}

void Config::setPoint(const ConfigItem configValue, const QPoint &point) const {
    this->set(configValue, QVariant(point));
}
