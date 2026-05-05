//
// Created by redrch in NuitkaStudio on 2026/4/27.
// Apache 2.0 license
//

#ifndef STATUS_BAR_STYLE_H
#define STATUS_BAR_STYLE_H

#include <QProxyStyle>

class StatusBarStyle : public QProxyStyle {

public:
    using QProxyStyle::QProxyStyle;

    void drawPrimitive(PrimitiveElement element, const QStyleOption* option,
                       QPainter* painter, const QWidget* widget = nullptr) const override;
};



#endif //STATUS_BAR_STYLE_H
