#include "KDSoapValue.h"
#include <QDebug>

uint qHash( const KDSoapValue& value ) { return qHash( value.name() ); }

QVariant KDSoapValueList::value(const QString &name) const
{
    const_iterator it = begin();
    const const_iterator e = end();
    for ( ; it != e; ++it) {
        const KDSoapValue& val = *it;
        if (val.name() == name)
            return val.value();
    }
    return QVariant();
}

QDebug operator <<(QDebug dbg, const KDSoapValue &value)
{
    dbg << value.name() << value.value();
    return dbg;
}

void KDSoapValueList::setType(const QString& nameSpace, const QString &type)
{
    m_type = qMakePair(nameSpace, type);
}

QString KDSoapValueList::typeNs() const
{
    return m_type.first;
}

QString KDSoapValueList::type() const
{
    return m_type.second;
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
