#pragma once

#include <QObject>

class ISensor : public QObject
{
    Q_OBJECT
public:
    explicit ISensor(QObject *parent = nullptr) : QObject(parent)
    {

    }

    QVariantList read() {}

signals:

};

