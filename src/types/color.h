//
// Created by redrch on 2026/2/21.
//

#ifndef COLOR_H
#define COLOR_H

#include <QMap>

inline QMap<QString, QString> color = {
    {"@BackgroundColor", "#f8f8f9"},
    {"@DividerColor", "#e8eaec"},
    {"@BorderColor", "#dcdee2"},
    {"@DisabledColor", "#c5c8ce"},
    {"@SubColor", "#808695"},
    {"@ContentColor", "#515a6e"},
    {"@TitleColor", "#17233d"},
    {"@InfoColor", "#2db7f5"},
    {"@WarningColor", "#ff9900"},
    {"@SuccessColor", "#19be6b"},
    {"@ErrorColor", "#ed4014"},
    {"@PrimaryColor", "#2d8cf0"},
    {"@LightPrimaryColor", "#5cadff"},
    {"@DarkPrimaryColor", "#2b85e4"}
};

namespace ColorName {
    inline const QString& background = "@BackgroundColor";
    inline const QString& divider = "@DividerColor";
    inline const QString& border = "@BorderColor";
    inline const QString& disabled = "@DisabledColor";
    inline const QString& sub = "@SubColor";
    inline const QString& content = "@ContentColor";
    inline const QString& title = "@TitleColor";
    inline const QString& info = "@InfoColor";
    inline const QString& warning = "@WarningColor";
    inline const QString& success = "@SuccessColor";
    inline const QString& error = "@ErrorColor";
    inline const QString& primary = "@PrimaryColor";
    inline const QString& lightPrimary = "@LightPrimaryColor";
    inline const QString& darkPrimary = "@DarkPrimaryColor";
}

#endif //COLOR_H
