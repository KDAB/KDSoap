/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPSERVERRAWXMLINTERFACE_H
#define KDSOAPSERVERRAWXMLINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QMap>
#include <QtCore/QObject>
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

    KDSoapServerRawXMLInterface(const KDSoapServerRawXMLInterface &other) = delete;
    KDSoapServerRawXMLInterface &operator=(const KDSoapServerRawXMLInterface &other) = delete;

    /**
     * Destructor
     */
    virtual ~KDSoapServerRawXMLInterface();

    /**
     * Called when starting to receive a new request.
     * @param requestType GET or POST
     * @param httpHeaders the map of http headers (keys have been lowercased since they are case insensitive)
     * @return true if you want this interface to handle the request, otherwise the usual processing via KDSoapServerObjectInterface::processRequest
     * will happen.
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
Q_DECLARE_INTERFACE(KDSoapServerRawXMLInterface, "com.kdab.KDSoap.ServerRawXMLInterface/1.0")
QT_END_NAMESPACE

#endif /* KDSOAPSERVERRAWXMLINTERFACE_H */
