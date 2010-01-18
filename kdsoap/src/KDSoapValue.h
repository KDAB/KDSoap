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

    QString name() const { return m_name; }

    // Can be any basic type, or a KDSoapValueList
    QVariant value() const { return m_value; }

private:
    QString m_name;
    QVariant m_value;
};

class KDSoapValueList : public QList<KDSoapValue>
{
public:

    void addArgument(const QString& argumentName, const QVariant& argumentValue)
    {
        append(KDSoapValue(argumentName, argumentValue));
    }

    // for the case where the name is unique
    QVariant value(const QString& name) const;
};

typedef QListIterator<KDSoapValue> KDSoapValueListIterator;

Q_DECLARE_METATYPE(KDSoapValueList)

#endif // KDSOAPVALUE_H
