//
// Created by redrch on 2025/12/5.
//

#include "config.h"

Config::Config() {
    this->configPath = "config.ini";
    this->configMap = new QMap<QString, QVariant>();
    this->configMap->insert("ConsoleInputEncoding", QVariant::fromValue<EncodingEnum>(EncodingEnum::utf8));
    this->configMap->insert("ConsoleOutputEncoding", QVariant::fromValue<EncodingEnum>(EncodingEnum::utf8));
    this->configMap->insert("PackTimerTriggerInterval", 100);
    this->configMap->insert("MaxPackLogCount", 20);
    this->configMap->insert("IsShowCloseWindow", QVariant(true));
    this->configMap->insert("IsHideOnClose", QVariant(true));
    this->configMap->insert("TempPath", QDir::tempPath() + "/NuitkaStudio");
    this->configMap->insert("NpfPath", "");
    this->configMap->insert("IsLightMode", QVariant(true));
    this->configMap->insert("FloatButtonSize", QVariant(QSize(25, 25)));
    this->configMap->insert("FloatButtonOriginalSize", QSize(25, 25));
    this->configMap->insert("FloatButtonPos", QVariant(QPoint(0, 0)));
    this->configMap->insert("IsFloatButtonLight", QVariant(true));

    this->configMap->insert("DefaultPythonPath", "C:/");
    this->configMap->insert("DefaultMainFilePath", "C:/");
    this->configMap->insert("DefaultOutputPath", "C:/");
    this->configMap->insert("DefaultIconPath", "C:/");
    this->configMap->insert("DefaultDataPath", "C:/");

    this->settings = new QSettings(this->configPath, QSettings::IniFormat);
}

Config::~Config() {
    delete this->settings;
}

void Config::writeConfig() {
    this->settings->beginGroup("General");
    this->settings->setValue("ConsoleInputEncoding",
                             encodingEnumToInt(
                                 this->getConfigEncodingEnum(SettingsEnum::ConsoleInputEncoding)));
    this->settings->setValue("ConsoleOutputEncoding",
                             encodingEnumToInt(
                                 this->getConfigEncodingEnum(SettingsEnum::ConsoleOutputEncoding)));
    this->settings->setValue("PackTimerTriggerInterval", this->getConfig(SettingsEnum::PackTimerTriggerInterval));
    this->settings->setValue("MaxPackLogCount", this->getConfig(SettingsEnum::MaxPackLogCount));
    this->settings->setValue("IsShowCloseWindow", this->getConfig(SettingsEnum::IsShowCloseWindow));
    this->settings->setValue("IsHideOnClose", this->getConfig(SettingsEnum::IsHideOnClose));
    this->settings->setValue("TempPath", this->getConfig(SettingsEnum::TempPath));
    this->settings->setValue("NpfPath", this->getConfig(SettingsEnum::NpfPath));
    this->settings->setValue("IsLightMode", this->getConfig(SettingsEnum::IsLightMode));
    this->settings->setValue("FloatButtonSize", this->getConfig(SettingsEnum::FloatButtonSize));
    this->settings->setValue("FloatButtonOriginalSize", this->getConfig(SettingsEnum::FloatButtonOriginalSize));
    this->settings->setValue("FloatButtonPos", this->getConfig(SettingsEnum::FloatButtonPos));
    this->settings->setValue("IsFloatButtonLight", this->getConfig(SettingsEnum::IsFloatButtonLight));
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->settings->setValue("DefaultPythonPath", this->getConfig(SettingsEnum::DefaultPythonPath));
    this->settings->setValue("DefaultMainFilePath", this->getConfig(SettingsEnum::DefaultMainFilePath));
    this->settings->setValue("DefaultOutputPath", this->getConfig(SettingsEnum::DefaultOutputPath));
    this->settings->setValue("DefaultIconPath", this->getConfig(SettingsEnum::DefaultIconPath));
    this->settings->setValue("DefaultDataPath", this->getConfig(SettingsEnum::DefaultDataPath));
    this->settings->endGroup();
}

void Config::readConfig() {
    this->settings->beginGroup("General");
    this->setConfigFromEncodingEnum(SettingsEnum::ConsoleInputEncoding,
                                    this->encodingEnumFromString(
                                        this->settings->value("ConsoleInputEncoding").toString()));
    this->setConfigFromEncodingEnum(SettingsEnum::ConsoleOutputEncoding,
                                    this->encodingEnumFromString(
                                        this->settings->value("ConsoleOutputEncoding").toString())
    );
    this->setConfig(SettingsEnum::PackTimerTriggerInterval, this->settings->value("PackTimerTriggerInterval").toInt());
    this->setConfig(SettingsEnum::MaxPackLogCount, this->settings->value("MaxPackLogCount").toInt());
    this->setConfig(SettingsEnum::IsShowCloseWindow, this->settings->value("IsShowCloseWindow").toBool());
    this->setConfig(SettingsEnum::IsHideOnClose, this->settings->value("IsHideOnClose").toBool());
    this->setConfig(SettingsEnum::TempPath, this->settings->value("TempPath").toString());
    this->setConfig(SettingsEnum::NpfPath, this->settings->value("NpfPath").toString());
    this->setConfig(SettingsEnum::IsLightMode, this->settings->value("IsLightMode").toBool());
    this->setConfig(SettingsEnum::FloatButtonSize, this->settings->value("FloatButtonSize").toSize());
    this->setConfig(SettingsEnum::FloatButtonOriginalSize, this->settings->value("FloatButtonOriginalSize").toSize());
    this->setConfig(SettingsEnum::FloatButtonPos, this->settings->value("FloatButtonPos").toPoint());
    this->setConfig(SettingsEnum::IsFloatButtonLight, this->settings->value("IsFloatButtonLight").toBool());
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->setConfig(SettingsEnum::DefaultPythonPath, this->settings->value("DefaultPythonPath").toString());
    this->setConfig(SettingsEnum::DefaultMainFilePath, this->settings->value("DefaultMainFilePath").toString());
    this->setConfig(SettingsEnum::DefaultOutputPath, this->settings->value("DefaultOutputPath").toString());
    this->setConfig(SettingsEnum::DefaultIconPath, this->settings->value("DefaultIconPath").toString());
    this->setConfig(SettingsEnum::DefaultDataPath, this->settings->value("DefaultDataPath").toString());
    this->settings->endGroup();
}

QVariant Config::getConfig(const QString &configValue) const {
    return this->configMap->value(configValue);
}

QVariant Config::getConfig(const SettingsEnum configValue) {
    return this->getConfig(this->settingsEnumToString(configValue));
}

QString Config::getConfigToString(const SettingsEnum configValue) {
    return this->getConfig(configValue).toString();
}

int Config::getConfigToInt(const SettingsEnum configValue) {
    return this->getConfig(configValue).toInt();
}

EncodingEnum Config::getConfigEncodingEnum(const SettingsEnum configValue) {
    return this->getConfig(configValue).value<EncodingEnum>();
}

bool Config::getConfigToBool(const SettingsEnum configValue) {
    return this->getConfig(configValue).toBool();
}

QSize Config::getConfigToSize(SettingsEnum configValue) {
    return this->getConfig(configValue).toSize();
}

QPoint Config::getConfigToPoint(SettingsEnum configValue) {
    return this->getConfig(configValue).toPoint();
}

void Config::setConfig(const QString &configValue, const QVariant &value) const {
    this->configMap->insert(configValue, value);
}

void Config::setConfig(const SettingsEnum configValue, const QVariant &value) {
    return this->setConfig(this->settingsEnumToString(configValue), value);
}

void Config::setConfigFromString(SettingsEnum configValue, const QString &string) {
    this->setConfig(configValue, QVariant(string));
}

void Config::setConfigFromInt(SettingsEnum configValue, int value) {
    this->setConfig(configValue, QVariant(value));
}

void Config::setConfigFromEncodingEnum(SettingsEnum configValue, EncodingEnum encodingValue) {
    this->setConfig(configValue, QVariant::fromValue<EncodingEnum>(encodingValue));
}

void Config::setConfigFromBool(SettingsEnum configValue, bool value) {
    this->setConfig(configValue, QVariant(value));
}

QString Config::settingsEnumToString(SettingsEnum enumValue) {
    QMetaEnum metaEnum = QMetaEnum::fromType<SettingsEnum>();
    QString string = QString::fromUtf8(metaEnum.valueToKey(static_cast<int>(enumValue)));
    return string;
}

void Config::setConfigFromSize(SettingsEnum configValue, const QSize &size) {
    this->setConfig(configValue, QVariant(size));
}

void Config::setConfigFromPoint(SettingsEnum configValue, const QPoint &point) {
    this->setConfig(configValue, QVariant(point));
}

SettingsEnum Config::settingsEnumFromString(const QString &string) {
    QMetaEnum metaEnum = QMetaEnum::fromType<SettingsEnum>();
    int value = metaEnum.keyToValue(string.toUtf8().constData());
    if (value == -1) {
        Logger::warn(QString("值%1在枚举SettingsEnum中不存在").arg(string));
        return SettingsEnum::NONE;
    }

    return static_cast<SettingsEnum>(value);
}

QString Config::encodingEnumToString(EncodingEnum enumValue) {
    switch (enumValue) {
        case EncodingEnum::utf8:
            return QStringLiteral("0");
        case EncodingEnum::gbk:
            return QStringLiteral("1");
        case EncodingEnum::ascii:
            return QStringLiteral("2");
        default:
            throw std::runtime_error(
                "toCode: unexpected EncodingEnum value: " + std::to_string(static_cast<int>(enumValue)));
    }
}

int Config::encodingEnumToInt(EncodingEnum enumValue) {
    switch (enumValue) {
        case EncodingEnum::utf8:
            return 0;
        case EncodingEnum::gbk:
            return 1;
        case EncodingEnum::ascii:
            return 2;
        default:
            throw std::runtime_error(
                "toCode: unexpected EncodingEnum value: " + std::to_string(static_cast<int>(enumValue)));
    }
}

EncodingEnum Config::encodingEnumFromString(const QString &string) {
    if (string == "0") {
        return EncodingEnum::utf8;
    } else if (string == "1") {
        return EncodingEnum::gbk;
    } else if (string == "2") {
        return EncodingEnum::ascii;
    } else {
        throw std::runtime_error("'Encoding Enum' value error.Please use >= 0 and <= 2 value");
    }
}

EncodingEnum Config::encodingEnumFromInt(int value) {
    if (value == 0) {
        return EncodingEnum::utf8;
    } else if (value == 1) {
        return EncodingEnum::gbk;
    } else if (value == 2) {
        return EncodingEnum::ascii;
    } else {
        throw std::runtime_error("'Encoding Enum' value error.Please use >= 0 and <= 2 value");
    }
}
