//
// Created by redrch on 2025/12/30.
//

#ifndef NUITKASTUDIO_ABOUT_WINDOW_H
#define NUITKASTUDIO_ABOUT_WINDOW_H

#include <QVBoxLayout>
#include <ElaDialog.h>
#include <ElaImageCard.h>
#include <ElaText.h>
#include <ElaTheme.h>

#include "app_config.h"

class AboutWindow : public ElaDialog {
Q_OBJECT

public:
    explicit AboutWindow(QWidget *parent = nullptr);

    ~AboutWindow() override;

private:

};


#endif //NUITKASTUDIO_ABOUT_WINDOW_H
