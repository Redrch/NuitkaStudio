//
// Created by redrch in NuitkaStudio on 2026/4/27.
// Apache 2.0 license
//

#include "status_bar_style.h"

void StatusBarStyle::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const {
    // 拦截“状态栏项框架”
    if (element == QStyle::PE_FrameStatusBarItem) {
        return;
    }

    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

