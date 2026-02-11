//
// Created by redrch on 2026/2/11.
//

#ifndef COMPRESS_H
#define COMPRESS_H

#include <quazip/JlCompress.h>
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>

#include "logger.h"

class Compress {
public:
    Compress();
    explicit Compress(const QString& zipPath);
    ~Compress();

    QByteArray readZip(const QString& filepath);
    void writeZip(const QString& filepath, const QByteArray& data);

    static void extractZip(const QString& zipPath, const QString& outputPath);
    static void extractFile(const QString& filePath, const QString& zipPath, const QString& outputPath);
    static void extractFiles(const QStringList& fileList, const QString& zipPath, const QString& outputPath);
    static bool compressDir(const QString& contentPath, const QString& zipPath);
    static bool compressFile(const QString& contentPath, const QString& zipPath);
    static bool compressFiles(const QStringList& contentList, const QString& zipPath);

private:
    QuaZip* zip;
};


#endif //COMPRESS_H
