//
// Created by redrch on 2025/12/5.
//

#ifndef NUITKASTUDIO_CONFIG_H
#define NUITKASTUDIO_CONFIG_H

#include <QString>
#include <QVariant>

#include <QDebug>

#include <QSettings>

enum class EncodingEnum {
    utf8 = 0,
    gbk = 1,
    ascii = 2
};

// Singleton class
class Config {
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

    // Getter and setter
    const QString &getDefaultPythonPath() const { return defaultPythonPath; }
    void setDefaultPythonPath(const QString &v) { defaultPythonPath = v; }

    const QString getDefaultMainFilePath() const { return defaultMainFilePath; }
    void setDefaultMainFilePath(const QString &v) { defaultMainFilePath = v; }

    const QString &getDefaultOutputPath() const { return defaultOutputPath; }
    void setDefaultOutputPath(const QString &v) { defaultOutputPath = v; }

    const QString &getDefaultIconPath() const { return defaultIconPath; }
    void setDefaultIconPath(const QString &v) { defaultIconPath = v; }

    const QString &getDefaultDataPath() const { return defaultDataPath; }
    void setDefaultDataPath(const QString &v) { defaultDataPath = v; }

    EncodingEnum getConsoleInputEncoding() const { return consoleInputEncoding; }
    void setConsoleInputEncoding(EncodingEnum e) { consoleInputEncoding = e; }

    EncodingEnum getConsoleOutputEncoding() const { return consoleOutputEncoding; }
    void setConsoleOutputEncoding(EncodingEnum e) { consoleOutputEncoding = e; }

    const QString& getConfigPath() const { return configPath; }

private:
    QSettings* settings;

    QString configPath;

    QString defaultPythonPath;
    QString defaultMainFilePath;
    QString defaultOutputPath;
    QString defaultIconPath;
    QString defaultDataPath;

    EncodingEnum consoleInputEncoding;
    EncodingEnum consoleOutputEncoding;
};


#endif //NUITKASTUDIO_CONFIG_H
