//
// Created by redrch in NuitkaStudio on 2026/5/4.
// Apache 2.0 license
//

#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <QPixmap>

class ImageProcessor : public QObject {
    Q_OBJECT

public:
    static QPixmap processImage(const QPixmap& source, qreal opacity, int blurRadius);
    static QPixmap applyBlur(const QPixmap& source, int radius);
};



#endif //IMAGE_PROCESSOR_H
