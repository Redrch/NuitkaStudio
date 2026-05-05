//
// Created by redrch in NuitkaStudio on 2026/5/4.
// Apache 2.0 license
//

#include "image_processor.h"
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsBlurEffect>

QPixmap ImageProcessor::processImage(const QPixmap &source, qreal opacity, int blurRadius) {
    if (source.isNull()) return QPixmap();

    // 模糊处理
    QPixmap blurred = applyBlur(source, blurRadius);

    // 不透明度
    QPixmap finalPixmap(blurred.size());
    finalPixmap.fill(Qt::transparent);

    QPainter painter(&finalPixmap);
    painter.setOpacity(opacity); // 应用不透明度
    painter.drawPixmap(0, 0, blurred);
    painter.end();

    return finalPixmap;
}

QPixmap ImageProcessor::applyBlur(const QPixmap &source, int radius) {
    if (radius <= 0 || source.isNull()) return source;

    // 定义边缘扩展距离
    int margin = radius;
    QSize originalSize = source.size();
    QSize expandedSize = originalSize + QSize(margin * 2, margin * 2);

    // 创建画布
    QPixmap expandedPixmap(expandedSize);
    expandedPixmap.fill(Qt::transparent);
    QPainter painter(&expandedPixmap);

    painter.drawPixmap(0, 0, margin, margin, source.copy(0, 0, 1, 1).scaled(margin, margin)); // 左上
    painter.drawPixmap(margin + originalSize.width(), 0, margin, margin, source.copy(originalSize.width() - 1, 0, 1, 1).scaled(margin, margin)); // 右上
    painter.drawPixmap(0, margin + originalSize.height(), margin, margin, source.copy(0, originalSize.height() - 1, 1, 1).scaled(margin, margin)); // 左下
    painter.drawPixmap(margin + originalSize.width(), margin + originalSize.height(), margin, margin, source.copy(originalSize.width() - 1, originalSize.height() - 1, 1, 1).scaled(margin, margin)); // 右下

    // 四边
    painter.drawPixmap(margin, 0, originalSize.width(), margin, source.copy(0, 0, originalSize.width(), 1).scaled(originalSize.width(), margin)); // 上
    painter.drawPixmap(margin, margin + originalSize.height(), originalSize.width(), margin, source.copy(0, originalSize.height() - 1, originalSize.width(), 1).scaled(originalSize.width(), margin)); // 下
    painter.drawPixmap(0, margin, margin, originalSize.height(), source.copy(0, 0, 1, originalSize.height()).scaled(margin, originalSize.height())); // 左
    painter.drawPixmap(margin + originalSize.width(), margin, margin, originalSize.height(), source.copy(originalSize.width() - 1, 0, 1, originalSize.height()).scaled(margin, originalSize.height())); // 右

    // 绘制四个角落及边缘
    painter.drawPixmap(0, 0, margin, margin, source.copy(0, 0, 1, 1).scaled(margin, margin)); // 左上
    painter.drawPixmap(margin, 0, originalSize.width(), margin, source.copy(0, 0, originalSize.width(), 1).scaled(originalSize.width(), margin)); // 上
    painter.drawPixmap(margin + originalSize.width(), 0, margin, margin, source.copy(originalSize.width() - 1, 0, 1, 1).scaled(margin, margin)); // 右上

    // 绘制原图
    painter.drawPixmap(margin, margin, source);

    painter.end();

    // 应用模糊
    QGraphicsScene scene;
    QGraphicsPixmapItem item;
    item.setPixmap(expandedPixmap);

    QGraphicsBlurEffect* blur = new QGraphicsBlurEffect();
    blur->setBlurRadius(radius);
    blur->setBlurHints(QGraphicsBlurEffect::QualityHint);
    item.setGraphicsEffect(blur);

    scene.addItem(&item);

    // 渲染并裁剪
    QPixmap result(expandedSize);
    result.fill(Qt::transparent);
    QPainter resPainter(&result);
    scene.render(&resPainter, QRectF(), QRectF(0, 0, expandedSize.width(), expandedSize.height()));
    resPainter.end();

    return result.copy(margin, margin, originalSize.width(), originalSize.height());
}


