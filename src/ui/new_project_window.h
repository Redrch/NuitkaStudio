//
// Created by redrch on 2026/1/3.
//

#ifndef NEW_PROJECT_WINDOW_H
#define NEW_PROJECT_WINDOW_H

#include <QWidget>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>

#include "../utils/logger.h"

enum class InterpreterType {
    Python,
    Virtualenv,
    UV
};

QT_BEGIN_NAMESPACE

namespace Ui {
    class NewProjectWindow;
}

QT_END_NAMESPACE

class NewProjectWindow : public QWidget {
    Q_OBJECT

public:
    explicit NewProjectWindow(QWidget *parent = nullptr);

    ~NewProjectWindow() override;

private:
    Ui::NewProjectWindow *ui;

    QString projectPath;
    QString projectName;

    InterpreterType interpreterType;
    QString pythonPath;
    QString virtualenvPath;
    QString uvPath;


    void connectPath();

private slots:
    void onPyTypeComboBoxCurrentIndexChanged(int index);
    void newProject();
};


#endif //NEW_PROJECT_WINDOW_H
