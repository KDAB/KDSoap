/*
    This file is part of KDE Schema Parser

    SPDX-FileCopyrightText: 2005 Tobias Koenig <tokoe@kde.org>
                       based on wsdlpull parser by Vivek Krishna

    SPDX-License-Identifier: MIT
 */

#include "KDQName.h"
#include <QDebug>

#include <KDSoapValue.h>

KDQName::KDQName()
{
}

KDQName::KDQName(const QString &name)
{
    parse(name);
}

KDQName::KDQName(const QString &nameSpace, const QString &localName)
    : mNameSpace(nameSpace)
    , mLocalName(localName)
{
    Q_ASSERT(!localName.contains(QLatin1Char(':')));
}

void KDQName::operator=(const QString &name)
{
    parse(name);
}

QString KDQName::localName() const
{
    return mLocalName;
}

QString KDQName::prefix() const
{
    return mPrefix;
}

QString KDQName::qname() const
{
    if (mPrefix.isEmpty())
        return mLocalName;
    else
        return mPrefix + QLatin1Char(':') + mLocalName;
}

void KDQName::setNameSpace(const QString &nameSpace)
{
    mNameSpace = nameSpace;
}

QString KDQName::nameSpace() const
{
    return mNameSpace;
}

bool KDQName::operator==(const KDQName &qname) const
{
    return (qname.nameSpace() == mNameSpace && qname.localName() == mLocalName);
}

bool KDQName::operator!=(const KDQName &qname) const
{
    return !operator==(qname);
}

bool KDQName::isEmpty() const
{
    return (mNameSpace.isEmpty() && mLocalName.isEmpty());
}

KDQName KDQName::fromSoapValue(const KDSoapValue &value)
{
    KDQName qname;
    qname.parse(value.value().toString());

    QXmlStreamNamespaceDeclarations decls = value.environmentNamespaceDeclarations();
    for (int i = 0; i < decls.count(); ++i) {
        const QXmlStreamNamespaceDeclaration &decl = decls.at(i);
        if (decl.prefix() == qname.prefix()) {
            qname.setNameSpace(decl.namespaceUri().toString());
        }
    }

    return qname;
}

KDSoapValue KDQName::toSoapValue(const QString &name, const QString &typeNameSpace, const QString &typeName) const
{
    KDSoapValue value = KDSoapValue(name, qname(), typeNameSpace, typeName);
    if (!mPrefix.isEmpty() && !mNameSpace.isEmpty()) {
        QXmlStreamNamespaceDeclaration decl(mPrefix, mNameSpace);
        value.addNamespaceDeclaration(decl);
    }
    return value;
}

void KDQName::parse(const QString &str)
{
    int pos = str.indexOf(QLatin1Char(':'));
    if (pos != -1) {
        mPrefix = str.left(pos);
        mLocalName = str.mid(pos + 1);
    } else {
        mLocalName = str;
    }
    Q_ASSERT(!mLocalName.contains(QLatin1Char(':')));
}

QDebug operator<<(QDebug dbg, const KDQName &qn)
{
    if (!qn.nameSpace().isEmpty())
        dbg << "(" << qn.nameSpace() << "," << qn.localName() << ")";
    else
        dbg << qn.qname();
    return dbg;
}
