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
    this->zipPath = zipPath;
    zip->setFileNameCodec("UTF-8");
}

Compress::~Compress() {
    this->zip->close();
    delete this->zip;
}

void Compress::setZipPath(const QString& zipPath) {
    this->zip->setZipName(zipPath);
    this->zipPath = zipPath;
}

QString Compress::getZipPath() const {
    return this->zip->getZipName();
}

void Compress::writeZip(const QString &internalPath, const QByteArray& data) {
    if (!this->zip->open(QuaZip::mdAppend)) {
        Logger::warn(QString("Compress::writeZip: Error opening zip file: ") + this->zipPath);
        return;
    }

    QuaZipFile outFile(this->zip);
    if (outFile.open(QIODevice::WriteOnly, QuaZipNewInfo(internalPath))) {
        outFile.write(data);
        outFile.close();
        this->zip->close();
    } else {
        Logger::warn(QString("Compress::writeZip: Error opening zip file: ") + this->zipPath);
    }
}

QByteArray Compress::readZip(const QString &internalPath) {
    if (!QFile::exists(this->zipPath)) {
        Logger::warn(QString("Compress::readZip: File does not exist: ") + this->zipPath);
        return {};
    }
    if (!this->zip->open(QuaZip::mdUnzip)) {
        Logger::warn(QString("Compress::readZip: Error opening zip file: ") + this->zipPath);
        return {};
    }

    if (!this->zip->setCurrentFile(internalPath)) {
        Logger::warn("Compress::readZip: Cannot find file in zip: " + internalPath);
        this->zip->close();
        return {};
    }

    QuaZipFile inFile(zip);
    if (inFile.open(QIODevice::ReadOnly)) {
        QByteArray data = inFile.readAll();
        inFile.close();
        this->zip->close();
        return data;
    }
    Logger::warn(QString("Compress::readZip: Error opening zip file: ") + this->zipPath);
    return {};
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

void Compress::createEmptyZip(const QString &zipPath) {
    QuaZip zip(zipPath);
    if (zip.open(QuaZip::mdCreate)) {
        zip.close();
    } else {
        Logger::warn("Compress::createEmptyZip: Error opening zip file: " + zipPath);
    }
}
