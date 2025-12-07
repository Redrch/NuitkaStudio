//
// Created by redrch on 2025/12/5.
//

#include "config.h"

Config::Config() {
    this->configPath = "config.ini";

    this->settings = new QSettings(this->configPath, QSettings::IniFormat);

    this->consoleInputEncoding = EncodingEnum::utf8;
    this->consoleOutputEncoding = EncodingEnum::utf8;

    this->configPath = QString(this->configPath);
    this->defaultPythonPath = QString("C:/");
    this->defaultMainFilePath = QString("C:/");
    this->defaultIconPath = QString("C:/");
}

Config::~Config() {
    delete this->settings;
}

void Config::writeConfig() {
    this->settings->beginGroup("General");
    this->settings->setValue("consoleInputEncoding",
                             QVariant(this->encodingEnumToString(this->getConsoleInputEncoding())));
    this->settings->setValue("consoleOutputEncoding",
                             QVariant(this->encodingEnumToString(this->getConsoleOutputEncoding())));
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->settings->setValue("defaultPythonPath", QVariant(this->getDefaultPythonPath()));
    this->settings->setValue("defaultMainFilePath", QVariant(this->getDefaultMainFilePath()));
    this->settings->setValue("defaultOutputPath", QVariant(this->getDefaultOutputPath()));
    this->settings->setValue("defaultIconPath", QVariant(this->getDefaultIconPath()));
    this->settings->endGroup();
}

void Config::readConfig() {
    this->settings->beginGroup("General");
    this->setConsoleInputEncoding(
            this->encodingEnumFromString(this->settings->value("consoleInputEncoding").toString()));
    this->setConsoleOutputEncoding(
            this->encodingEnumFromString(this->settings->value("consoleOutputEncoding").toString()));
    this->settings->endGroup();

    this->settings->beginGroup("DefaultPath");
    this->setDefaultPythonPath(this->settings->value("defaultPythonPath").toString());
    this->setDefaultMainFilePath(this->settings->value("defaultMainFilePath").toString());
    this->setDefaultOutputPath(this->settings->value("defaultOutputPath").toString());
    this->setDefaultIconPath(this->settings->value("defaultIconPath").toString());
    this->settings->endGroup();
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

