//
// Created by redrch on 2025/12/30.
//

#ifndef NUITKASTUDIO_ABOUT_WINDOW_H
#define NUITKASTUDIO_ABOUT_WINDOW_H

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

// 手动定义缺失的宏（Windows 11 特有属性）
#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

#ifndef DWMWA_TEXT_COLOR
#define DWMWA_TEXT_COLOR 36
#endif

#include <QVBoxLayout>
#include <ElaDialog.h>
#include <ElaImageCard.h>
#include <ElaText.h>
#include <ElaTheme.h>

#include "app_config.h"

class AboutWindow : public QDialog {
Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);

    ~AboutWindow() override;
};


#endif //NUITKASTUDIO_ABOUT_WINDOW_H
