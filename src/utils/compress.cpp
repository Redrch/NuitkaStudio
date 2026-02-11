//
// Created by redrch on 2026/2/11.
//

#include "compress.h"

Compress::Compress() {
    zip = new QuaZip();
    zip->setFileNameCodec("UTF-8");
}

Compress::Compress(const QString& zipPath) {
    this->zip = new QuaZip(zipPath);
    zip->setFileNameCodec("UTF-8");
}

Compress::~Compress() {
    delete this->zip;
}

void Compress::writeZip(const QString &filepath, const QByteArray& data) {
    if (!QFile::exists(filepath)) {
        Logger::warn(QString("Compress::writeZip: File does not exist: ") + filepath);
        return;
    }
    if (!this->zip->open(QuaZip::mdAppend)) {
        Logger::warn(QString("Compress::writeZip: Error opening zip file: ") + filepath);
        return;
    }

    zip->setCurrentFile(filepath);
    QuaZipFile zipFile(zip);
    zipFile.write(data);

    zipFile.close();
}

QByteArray Compress::readZip(const QString &filepath) {
    if (!QFile::exists(filepath)) {
        Logger::warn(QString("Compress::writeZip: File does not exist: ") + filepath);
        return {};
    }
    if (!this->zip->open(QuaZip::mdUnzip)) {
        Logger::warn(QString("Compress::writeZip: Error opening zip file: ") + filepath);
        return {};
    }

    zip->setCurrentFile(filepath);
    QuaZipFile zipFile(zip);
    return zipFile.readAll();
}

void Compress::extractZip(const QString& zipPath, const QString& outputPath) {
    QStringList files = JlCompress::extractDir(zipPath, outputPath);
    if (files.isEmpty()) {
        Logger::warn("Compress::extractZip: File does not exist or file content is null: " + zipPath);
        return;
    }
}

void Compress::extractFile(const QString &filePath, const QString &zipPath, const QString &outputPath) {
    QString file = JlCompress::extractFile(zipPath, filePath, outputPath);
    if (file.isEmpty()) {
        Logger::warn("Compress::extractFile: File does not exist or file content is null: " + zipPath);
        return;
    }
}

void Compress::extractFiles(const QStringList &fileList, const QString &zipPath, const QString &outputPath) {
    QStringList list = JlCompress::extractFiles(zipPath, fileList, outputPath);
    if (list.isEmpty()) {
        Logger::warn("Compress::extractFiles: File does not exist or list is empty: " + zipPath);
        return;
    }
}

bool Compress::compressDir(const QString& contentPath, const QString& zipPath) {
    bool success = JlCompress::compressDir(zipPath, contentPath, true);
    if (!success) {
        Logger::warn("Compress::compressZip: Error compressing zip file: " + zipPath);
    }
    return success;
}

bool Compress::compressFile(const QString &contentPath, const QString &zipPath) {
    bool success = JlCompress::compressFile(contentPath, zipPath);
    if (!success) {
        Logger::warn("Compress::compressFile: Error compressing zip file: " + zipPath);
    }
    return success;
}

bool Compress::compressFiles(const QStringList &contentList, const QString &zipPath) {
    bool success = JlCompress::compressFiles(zipPath, contentList);
    if (!success) {
        Logger::warn("Compress::compressFiles: Error compressing zip files: " + zipPath);
    }
    return success;
}

