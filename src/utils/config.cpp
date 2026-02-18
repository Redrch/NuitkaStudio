//
// Created by redrch on 2025/12/5.
//

#include "config.h"

Config::Config() {
    this->configPath = "config.ini";
    this->configMap = new QMap<QString, QVariant>();
    this->settings = new QSettings(this->configPath, QSettings::IniFormat);
}

Config::~Config() {
    delete this->settings;
}

void Config::writeConfig() {
    this->settings->beginGroup("General");
    this->settings->setValue("consoleInputEncoding", this->getConfig(SettingsEnum::ConsoleInputEncoding));
    this->settings->setValue("consoleOutputEncoding", this->getConfig(SettingsEnum::ConsoleOutputEncoding));
    this->settings->setValue("packTimerTriggerInterval", this->getConfig(SettingsEnum::PackTimerTriggerInterval));
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->settings->setValue("defaultPythonPath", this->getConfig(SettingsEnum::DefaultPythonPath));
    this->settings->setValue("defaultMainFilePath", this->getConfig(SettingsEnum::DefaultMainFilePath));
    this->settings->setValue("defaultOutputPath", this->getConfig(SettingsEnum::DefaultOutputPath));
    this->settings->setValue("defaultIconPath", this->getConfig(SettingsEnum::DefaultIconPath));
    this->settings->setValue("defaultDataPath", this->getConfig(SettingsEnum::DefaultDataPath));
    this->settings->setValue("tempPath", this->getConfig(SettingsEnum::TempPath));
    this->settings->endGroup();
}

void Config::readConfig() {
    this->settings->beginGroup("General");
    this->setConfigFromEncodingEnum(SettingsEnum::ConsoleInputEncoding,
                                    this->encodingEnumFromString(
                                        this->settings->value("consoleInputEncoding").toString()));
    this->setConfigFromEncodingEnum(SettingsEnum::ConsoleOutputEncoding,
                                    this->encodingEnumFromString(
                                        this->settings->value("consoleOutputEncoding").toString())
    );
    this->setConfig(SettingsEnum::PackTimerTriggerInterval, this->settings->value("packTimerTriggerInterval").toInt());
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->setConfig(SettingsEnum::DefaultPythonPath, this->settings->value("defaultPythonPath").toString());
    this->setConfig(SettingsEnum::DefaultMainFilePath, this->settings->value("defaultMainFilePath").toString());
    this->setConfig(SettingsEnum::DefaultOutputPath, this->settings->value("defaultOutputPath").toString());
    this->setConfig(SettingsEnum::DefaultIconPath, this->settings->value("defaultIconPath").toString());
    this->setConfig(SettingsEnum::DefaultDataPath, this->settings->value("defaultDataPath").toString());
    this->setConfig(SettingsEnum::TempPath, this->settings->value("tempPath").toString());
    this->settings->endGroup();
}

QVariant Config::getConfig(const QString &configValue) const {
    return this->configMap->value(configValue);
}

QVariant Config::getConfig(const SettingsEnum configValue) {
    return this->getConfig(this->settingsEnumToString(configValue));
}

QString Config::getConfigToString(SettingsEnum configValue) {
    return this->getConfig(configValue).toString();
}

int Config::getConfigToInt(SettingsEnum configValue) {
    return this->getConfig(configValue).toInt();
}

EncodingEnum Config::getConfigEncodingEnum(SettingsEnum configValue) {
    return this->getConfig(configValue).value<EncodingEnum>();
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

QString Config::settingsEnumToString(SettingsEnum enumValue) {
    QMetaEnum metaEnum = QMetaEnum::fromType<SettingsEnum>();
    QString string = QString::fromUtf8(metaEnum.valueToKey(static_cast<int>(enumValue)));
    return string;
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
        qDebug() << string;
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
        qDebug() << value;
        throw std::runtime_error("'Encoding Enum' value error.Please use >= 0 and <= 2 value");
    }
}
