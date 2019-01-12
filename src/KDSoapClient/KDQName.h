/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>
                       based on wsdlpull parser by Vivek Krishna

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 */

#ifndef KDQNAME_H
#define KDQNAME_H

#include <QString>
#include <QList>
#include <QHash>

class KDSoapValue;

#include "KDSoapGlobal.h"

class KDSOAP_EXPORT KDQName
{
  public:
    typedef QList<KDQName> List;

    KDQName();

    // Create a KDQName with prefix+localname
    explicit KDQName( const QString &name );

    // Create a KDQName with namespace+localname
    KDQName( const QString &nameSpace, const QString &localName );

    void operator=( const QString &name );

    QString localName() const;
    QString prefix() const;
    QString qname() const;

    void setNameSpace( const QString &nameSpace );
    QString nameSpace() const;

    bool operator==( const KDQName& ) const;
    bool operator!=( const KDQName& ) const;

    bool isEmpty() const;

    /**
     * Creates a KDQName from a KDSoapValue.
     */
    static KDQName fromSoapValue(const KDSoapValue &value);

    /**
     * Returns a KDSoapValue representation.
     */
    KDSoapValue toSoapValue(const QString &name, const QString &typeNameSpace = QString(), const QString &typeName = QString()) const;

  private:
    void parse( const QString& );

    QString mNameSpace;
    QString mLocalName;
    QString mPrefix;
};

Q_DECLARE_METATYPE(KDQName)

inline uint qHash(const KDQName& qn) { return qHash(qn.nameSpace())^qHash(qn.localName()); }

QDebug operator<<(QDebug dbg, const KDQName &qn);

#endif
