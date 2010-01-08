#ifndef KDSOAPVALUE_H
#define KDSOAPVALUE_H

#include <QString>
#include <QVariant>
#include <QList>

struct KDSoapValue
{
    KDSoapValue(const QString& n, const QVariant& v)
        : name(n), value(v) {}

    QString name; // not necessarily unique!
    QVariant value;
};

class KDSoapValueList : public QList<KDSoapValue>
{
public:

    // for the case where the name is unique
    QVariant findByName(const QString& name) const {
        const_iterator it = begin();
        const const_iterator e = end();
        for ( ; it != e; ++it) {
            const KDSoapValue& val = *it;
            if (val.name == name)
                return val.value;
        }
        return QVariant();
    }
};

typedef QListIterator<KDSoapValue> KDSoapValueListIterator;

#endif // KDSOAPVALUE_H
