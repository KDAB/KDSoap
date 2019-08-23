/*
    This file is part of KDE.

    Copyright (c) 2005 Tobias Koenig <tokoe@kde.org>

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

#include "nsmanager.h"
#include "parsercontext.h"

#include <QDebug>
#include <QDomElement>

// maybe port to QXmlNamespaceSupport?

NSManager::NSManager()
    : mContext(nullptr), mParentManager(nullptr)
{
}

NSManager::NSManager( ParserContext* context, const QDomElement& child )
{
    mContext = context;
    mParentManager = context->namespaceManager();
    mMap = mParentManager->mMap;
    mCurrentNamespace = mParentManager->mCurrentNamespace;
    enterChild(child);
    mContext->setNamespaceManager(this);
}

NSManager::~NSManager()
{
    // Restore parent namespaces
    if (mContext) {
        mContext->setNamespaceManager(mParentManager);

        // Remember the prefixes used for the namespaces, even afterwards
        //qDebug() << this << "adding prefixes into parent manager" << mParentManager;
        mParentManager->addPrefixes( mMap );
    }
}

void NSManager::setCurrentNamespace( const QString& uri )
{
    mCurrentNamespace = uri;
}

void NSManager::setPrefix( const QString &prefix, const QString &uri )
{
    mMap.replace( prefix, uri );
}

QString NSManager::prefix( const QString &uri ) const
{
    // Note that it's allowed to have two prefixes for the same namespace uri.
    // So we just pick one.
    QString pref = mMap.key( uri ); // linear search
    if (pref.isEmpty() && uri != "http://schemas.xmlsoap.org/wsdl/") {
        qWarning() << "WARNING: No prefix found for" << uri;
    }
    return pref;
}

QString NSManager::uri( const QString &prefix ) const
{
    if (prefix.isEmpty())
        return mCurrentNamespace;
    return mMap.value( prefix );
}

void NSManager::splitName( const QString &qname, QString &prefix, QString &localname ) const
{
  int pos = qname.indexOf( QLatin1Char(':') );
  if ( pos != -1 ) {
    prefix = qname.left( pos );
    localname = qname.mid( pos + 1 );
  } else {
    prefix = QString();
    localname = qname;
  }
}

QString NSManager::fullName( const QString &nameSpace, const QString &localname ) const
{
  if ( prefix( nameSpace ).isEmpty() )
    return localname;
  else
    return prefix( nameSpace ) + QLatin1Char(':') + localname;
}

QString NSManager::fullName( const QName &name ) const
{
  return fullName( name.nameSpace(), name.localName() );
}

QStringList NSManager::prefixes() const
{
  return mMap.keys();
}

QMap<QString, QString> NSManager::prefixMap() const
{
    return mMap;
}

void NSManager::addPrefixes(const QMap<QString, QString> &prefixes)
{
    for (QMap<QString, QString>::const_iterator it = prefixes.constBegin() ; it != prefixes.constEnd() ; ++it) {
        const QString &prefix = it.key();
        const QString &ns = it.value();
        // Only write down this prefix if we don't have it yet
        // and if we don't have another prefix for this NS (this is mostly for backwards compat,
        // so that msexchange still uses TNS rather than 'M' or 'T'...)
        if (!mMap.contains(prefix) && mMap.key(ns).isEmpty())
            mMap.insert(prefix, ns);
    }
}

QStringList NSManager::soapEncNamespaces()
{
    return QStringList() << QLatin1String("http://schemas.xmlsoap.org/soap/encoding/")
                         << QLatin1String("http://www.w3.org/2003/05/soap-encoding");
}

QStringList NSManager::soapNamespaces()
{
    return QStringList() << QLatin1String("http://schemas.xmlsoap.org/wsdl/soap/")
                         << QLatin1String("http://schemas.xmlsoap.org/wsdl/soap12/");
}

QString NSManager::xmlNamespace()
{
    return QLatin1String("http://www.w3.org/XML/1998/namespace");
}

void NSManager::reset()
{
  mMap.clear();
}

void NSManager::dump() const
{
  QMap<QString, QString>::ConstIterator it;
  for ( it = mMap.begin(); it != mMap.end(); ++it ) {
    qDebug( "%s\t%s", qPrintable( it.key() ), qPrintable( it.value() ) );
  }
}

QString NSManager::nameSpace(const QDomElement &element) const
{
    if (!element.namespaceURI().isEmpty()) // namespace support was enabled in QDom: easy
        return element.namespaceURI();

    QString prefix, localname;
    splitName( element.tagName(), prefix, localname );
    return uri( prefix );
}

QString NSManager::localName(const QDomElement &element) const
{
    if (!element.namespaceURI().isEmpty()) // namespace support was enabled in QDom: easy
        return element.localName();

    QString prefix, localname;
    splitName( element.tagName(), prefix, localname );
    return localname;
}

void NSManager::enterChild(const QDomElement &element)
{
    const QDomNamedNodeMap attributes = element.attributes();
    for ( int i = 0; i < attributes.count(); ++i ) {
      QDomAttr attribute = attributes.item( i ).toAttr();
      if ( attribute.name().startsWith( QLatin1String("xmlns:") ) ) {
        QString prefix = attribute.name().mid( 6 );
        //if (prefix == "tns")
        //    qDebug() << this << "enterChild: setting tns to" << attribute.value();
        setPrefix( prefix, attribute.value() );
      } else if ( attribute.name() == QLatin1String("xmlns") ) {
        setCurrentNamespace( attribute.value() );
      }
    }
}
