//
// Created by redrch on 2026/1/16.
//

#pragma once
#ifndef PROJECTCONFIGTYPE_H
#define PROJECTCONFIGTYPE_H

#include <QVariant>
#include <QJsonObject>

#include "data_structs.h"

class ProjectConfigType {
public:
    ProjectConfigType() = default;

    ProjectConfigType(const QString &name, const QVariant &value) {
        this->itemName = name;
        this->itemValue = value;
    }

    ~ProjectConfigType() = default;

    [[nodiscard]] QString get_itemName() const {
        return itemName;
    }

    void set_itemName(const QString &item_name) {
        itemName = item_name;
    }

    [[nodiscard]] QVariant get_itemValue() const {
        return itemValue;
    }

    void set_itemValue(const QVariant &item_value) {
        itemValue = item_value;
    }

    [[nodiscard]] QString get_itemValueString() const {
        return itemValue.toString();
    }

    [[nodiscard]] bool get_itemValueBool() const {
        return itemValue.toBool();
    }

private:
    QString itemName;
    QVariant itemValue;
};

inline QDataStream &operator<<(QDataStream &out, const ProjectConfigType &item) {
    out << item.get_itemName() << item.get_itemValue();
    return out;
}

inline QDataStream &operator>>(QDataStream &in, ProjectConfigType &item) {
    QString name;
    QVariant value;
    in >> name >> value;
    item.set_itemName(name);
    item.set_itemValue(value);
    return in;
}


#endif //PROJECTCONFIGTYPE_H
