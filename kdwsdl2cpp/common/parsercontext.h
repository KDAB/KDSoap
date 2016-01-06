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

#ifndef PARSERCONTEXT_H
#define PARSERCONTEXT_H

#include <QUrl>
#include <kode_export.h>

class NSManager;
class MessageHandler;

/**
  A container class which is passed to XML parsing
  methods to provide additional contextual information.
 */
class KXMLCOMMON_EXPORT ParserContext
{
  public:
    /**
      Constructs a ParserContext.
     */
    ParserContext();

    /**
      Destroys the ParserContext.
     */
    virtual ~ParserContext();

    /**
      Sets the namespace handler for this context.
     */
    void setNamespaceManager( NSManager *manager );

    /**
      Returns the namespace handler of this context.
     */
    NSManager* namespaceManager() const;

    /**
      Sets the message handler for this context.
     */
    void setMessageHandler( MessageHandler *handler );

    /**
      Returns the message handler of this context.
     */
    MessageHandler* messageHandler() const;

    /**
      Sets the base URL where the document is located.
     */
    void setDocumentBaseUrl( const QUrl &url );

    /**
      Sets the base URL based on an actual document URL
      (i.e. set it to the parent directory)
     */
    void setDocumentBaseUrlFromFileUrl( const QUrl &url );

    /**
      Returns the document base url.
     */
    QUrl documentBaseUrl() const;

  private:
    NSManager *mNamespaceManager;
    MessageHandler *mMessageHandler;
    QUrl mDocumentBaseUrl;
};

#endif

