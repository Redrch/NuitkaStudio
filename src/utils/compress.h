//
// Created by redrch on 2026/2/11.
//

#ifndef COMPRESS_H
#define COMPRESS_H

#include <quazip/JlCompress.h>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "logger.h"
#include "../types/data_structs.h"

class Compress {
public:
    Compress();
    explicit Compress(const QString& zipPath);
    ~Compress();

    void setZipPath(const QString& zipPath);
    [[nodiscard]] QString getZipPath() const;
    QByteArray readZip(const QString &internalPath);
    void writeZip(const QString &internalPath, const QByteArray& data, QuaZip::Mode mode = QuaZip::mdCreate);
    void initZip();

    static void extractZip(const QString& zipPath, const QString& outputPath);
    static void extractFile(const QString& filePath, const QString& zipPath, const QString& outputPath);
    static void extractFiles(const QStringList& fileList, const QString& zipPath, const QString& outputPath);
    static bool compressDir(const QString& contentPath, const QString& zipPath);
    static bool compressFile(const QString& contentPath, const QString& zipPath);
    static bool compressFiles(const QStringList& contentList, const QString& zipPath);
    static void createEmptyZip(const QString& zipPath);

private:
    QuaZip* zip;
    QString zipPath;
};


#endif //COMPRESS_H
