#ifndef KDSOAPVALUE_H
#define KDSOAPVALUE_H

#include <QString>
#include <QVariant>
#include <QList>

class KDSoapValue
{
public:
    KDSoapValue(const QString& n, const QVariant& v)
        : m_name(n), m_value(v) {}

    // not necessarily unique!
    QString name() const { return m_name; }
    QVariant value() const { return m_value; }

private:
    QString m_name;
    QVariant m_value;
};

class KDSoapValueList : public QList<KDSoapValue>
{
public:

    // for the case where the name is unique
    QVariant value(const QString& name) const {
        const_iterator it = begin();
        const const_iterator e = end();
        for ( ; it != e; ++it) {
            const KDSoapValue& val = *it;
            if (val.name() == name)
                return val.value();
        }
        return QVariant();
    }
};

typedef QListIterator<KDSoapValue> KDSoapValueListIterator;

#endif // KDSOAPVALUE_H
