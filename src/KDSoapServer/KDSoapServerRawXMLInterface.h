/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#ifndef KDSOAPSERVERRAWXMLINTERFACE_H
#define KDSOAPSERVERRAWXMLINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QObject>
#include <QtCore/QMap>
class KDSoapRawXMLentication;
class KDSoapServerSocket;
class KDSoapServerRawXMLPrivate;

/**
 * Additional interface for processing incoming XML directly, without the use of KDSoapMessage.
 *
 * In addition to deriving from KDSoapServerObjectInterface, you can derive from
 * KDSoapServerRawXMLInterface in order to handle XML as it comes in.
 *
 * Use Q_INTERFACES(KDSoapServerRawXMLInterface) in your derived class (under Q_OBJECT)
 * so that Qt can discover the additional inheritance.
 *
 *
 * This can be useful for pre-processing invalid XML, or for handling very large requests
 * (received in chunks) in an incremental way, saving memory compared to the usual buffering
 * of the entire request.
 *
 * KDSoapServer will receive all HTTP headers, call newRequest(),
 * call processXML() for every chunk of XML, and finally call endRequest().
 * Use writeHTTP() or writeXML() (from KDSoapServerObjectInterface) to reply back.
 *
 * \since 1.5
 */
class KDSOAPSERVER_EXPORT KDSoapServerRawXMLInterface
{
public:
    /**
     * Constructor
     */
    KDSoapServerRawXMLInterface();

    /**
     * Destructor
     */
    virtual ~KDSoapServerRawXMLInterface();

    /**
     * Called when starting to receive a new request.
     * @param requestType GET or POST
     * @param httpHeaders the map of http headers (keys have been lowercased since they are case insensitive)
     * @return true if you want this interface to handle the request, otherwise the usual processing via KDSoapServerObjectInterface::processRequest will happen.
     */
    virtual bool newRequest(const QByteArray &requestType, const QMap<QByteArray, QByteArray> &httpHeaders)
    {
        Q_UNUSED(requestType);
        Q_UNUSED(httpHeaders);
        return false;
    }

    /**
     * Called with the chunks of XML data as they come in
     */
    virtual void processXML(const QByteArray &xmlChunk)
    {
        Q_UNUSED(xmlChunk);
    }

    /**
     * Called at the end of the request.
     * Use writeHTTP() or writeXML() (from KDSoapServerObjectInterface) to reply back.
     */
    virtual void endRequest()
    {
    }

private:
    KDSoapServerRawXMLPrivate *const d;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(KDSoapServerRawXMLInterface,
                    "com.kdab.KDSoap.ServerRawXMLInterface/1.0")
QT_END_NAMESPACE

#endif /* KDSOAPSERVERRAWXMLINTERFACE_H */
