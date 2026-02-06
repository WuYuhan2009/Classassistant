#ifndef BUTTONCONFIG_H
#define BUTTONCONFIG_H

#include <QString>
#include <QMetaType>

struct ButtonConfig
{
    QString name;
    QString icon;
    QString type;
    QString data;
    bool enabled;
    bool isDefault;

    ButtonConfig() : enabled(true), isDefault(false) {}
};

Q_DECLARE_METATYPE(ButtonConfig)

#endif
