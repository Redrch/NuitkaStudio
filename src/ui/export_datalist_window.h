//
// Created by redrch on 2025/12/12.
//

#ifndef NUITKASTUDIO_EXPORT_DATALIST_WINDOW_H
#define NUITKASTUDIO_EXPORT_DATALIST_WINDOW_H

#include <QWidget>
#include <QList>
#include <QFileDialog>

#include "../utils/config.h"
#include "../types/project_config_manager.h"
#include "../types/simname.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ExportDataListWindow; }
QT_END_NAMESPACE

class ExportDataListWindow : public QWidget {
Q_OBJECT

public:
    explicit ExportDataListWindow(QWidget *parent = nullptr);
    ~ExportDataListWindow() override;

    void updateUI();
private:
    Ui::ExportDataListWindow *ui;
};


#endif //NUITKASTUDIO_EXPORT_DATALIST_WINDOW_H
