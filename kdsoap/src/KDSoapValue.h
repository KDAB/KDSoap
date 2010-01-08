#ifndef KDSOAPVALUE_H
#define KDSOAPVALUE_H

#include <QString>
#include <QVariant>
#include <QList>

struct KDSoapValue
{
    KDSoapValue(const QString& n, const QVariant& v)
        : name(n), value(v) {}

    QString name;
    QVariant value;
};

typedef QList<KDSoapValue> KDSoapValueList;
typedef QListIterator<KDSoapValue> KDSoapValueListIterator;

#endif // KDSOAPVALUE_H
