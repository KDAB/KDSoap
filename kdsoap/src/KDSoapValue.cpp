#include "KDSoapValue.h"

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
