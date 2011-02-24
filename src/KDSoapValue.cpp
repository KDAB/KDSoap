#include "KDSoapValue.h"
#include <QDebug>

class KDSoapValue::Private : public QSharedData
{
public:
    Private() {}
    Private(const QString& n, const QVariant& v, const QString& typeNameSpace, const QString& typeName)
        : m_name(n), m_value(v), m_typeNamespace(typeNameSpace), m_typeName(typeName) {}

    QString m_name;
    QVariant m_value;
    QString m_typeNamespace;
    QString m_typeName;
    KDSoapValueList m_childValues;
};

uint qHash( const KDSoapValue& value ) { return qHash( value.name() ); }


KDSoapValue::KDSoapValue()
    : d(new Private)
{
}

KDSoapValue::KDSoapValue(const QString& n, const QVariant& v, const QString& typeNameSpace, const QString& typeName)
    : d(new Private(n, v, typeNameSpace, typeName))
{
}

KDSoapValue::KDSoapValue(const QString& n, const KDSoapValueList& children, const QString& typeNameSpace, const QString& typeName)
    : d(new Private(n, QVariant(), typeNameSpace, typeName))
{
    d->m_childValues = children;
}

KDSoapValue::~KDSoapValue()
{
}

KDSoapValue::KDSoapValue(const KDSoapValue& other)
    : d(other.d)
{
}



QString KDSoapValue::name() const
{
    return d->m_name;
}

QVariant KDSoapValue::value() const
{
    return d->m_value;
}

void KDSoapValue::setValue(const QVariant &value)
{
    d->m_value = value;
}

KDSoapValueList & KDSoapValue::childValues() const
{
    // I want to fool the QSharedDataPointer mechanism here...
    return const_cast<KDSoapValueList &>(d->m_childValues);
}


bool KDSoapValue::operator ==(const KDSoapValue &other) const
{
    return d == other.d;
}

////

QDebug operator <<(QDebug dbg, const KDSoapValue &value)
{
    dbg.space() << value.name() << value.value();
    if (!value.childValues().isEmpty()) {
        dbg << "<children>";
        KDSoapValueListIterator it(value.childValues());
        while (it.hasNext()) {
            const KDSoapValue& child = it.next();
            dbg << child;
        }
        dbg << "</children>";
    }
    if (!value.childValues().attributes().isEmpty()) {
        dbg << "<attributes>";
        QListIterator<KDSoapValue> it(value.childValues().attributes());
        while (it.hasNext()) {
            const KDSoapValue& child = it.next();
            dbg << child;
        }
        dbg << "</attributes>";
    }
    return dbg;
}

void KDSoapValue::setType(const QString& nameSpace, const QString &type)
{
    d->m_typeNamespace = nameSpace;
    d->m_typeName = type;
}

QString KDSoapValue::typeNs() const
{
    return d->m_typeNamespace;
}

QString KDSoapValue::type() const
{
    return d->m_typeName;
}

KDSoapValue KDSoapValueList::child(const QString &name) const
{
    const_iterator it = begin();
    const const_iterator e = end();
    for ( ; it != e; ++it) {
        const KDSoapValue& val = *it;
        if (val.name() == name)
            return val;
    }
    return KDSoapValue();
}

void KDSoapValueList::setArrayType(const QString& nameSpace, const QString &type)
{
    m_arrayType = qMakePair(nameSpace, type);
}

QString KDSoapValueList::arrayTypeNs() const
{
    return m_arrayType.first;
}

QString KDSoapValueList::arrayType() const
{
    return m_arrayType.second;
}

void KDSoapValueList::addArgument(const QString& argumentName, const QVariant& argumentValue, const QString& typeNameSpace, const QString& typeName)
{
    append(KDSoapValue(argumentName, argumentValue, typeNameSpace, typeName));
}
