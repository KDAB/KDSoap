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

#include "parsercontext.h"
#include "nsmanager.h"
#include <QDebug>

ParserContext::ParserContext()
  : mNamespaceManager( 0 ),
    mMessageHandler( 0 )
{
}

ParserContext::~ParserContext()
{
}

void ParserContext::setNamespaceManager( NSManager *manager )
{
  mNamespaceManager = manager;
  //qDebug() << "ParserContext now points to" << manager << "tns=" << manager->uri("tns");
}

NSManager* ParserContext::namespaceManager() const
{
  return mNamespaceManager;
}

void ParserContext::setMessageHandler( MessageHandler *handler )
{
  mMessageHandler = handler;
}

MessageHandler* ParserContext::messageHandler() const
{
  return mMessageHandler;
}

void ParserContext::setDocumentBaseUrl( const QUrl &url )
{
    mDocumentBaseUrl = url;
}

void ParserContext::setDocumentBaseUrlFromFileUrl( const QUrl &url )
{
    QString path = url.path();
    path.truncate( path.lastIndexOf('/') );
    QUrl newBaseUrl = url;
    newBaseUrl.setPath(path);
    //qDebug() << "New document base URL" << newBaseUrl;
    setDocumentBaseUrl( newBaseUrl );
}

QUrl ParserContext::documentBaseUrl() const
{
  return mDocumentBaseUrl;
}
