/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPSERVERCUSTOMVERBREQUESTINTERFACE_H
#define KDSOAPSERVERCUSTOMVERBREQUESTINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QMap>
#include <QtCore/QObject>
class KDSoapAuthentication;
class KDSoapServerSocket;

/**
 * Additional interface for handling custom verb request.
 *
 * In addition to deriving from KDSoapServerObjectInterface, you can derive from
 * KDSoapServerCustomVerbRequestInterface in order to handle custom verb HTTP requests.
 *
 * Use Q_INTERFACES(KDSoapServerCustomVerbRequestInterface) in your derived class (under Q_OBJECT)
 * so that Qt can discover the additional inheritance.
 *
 * \since 1.5
 */
class KDSOAPSERVER_EXPORT KDSoapServerCustomVerbRequestInterface
{
public:
    /**
     * Constructor
     */
    KDSoapServerCustomVerbRequestInterface();

    KDSoapServerCustomVerbRequestInterface(const KDSoapServerCustomVerbRequestInterface &other) = delete;
    KDSoapServerCustomVerbRequestInterface &operator=(const KDSoapServerCustomVerbRequestInterface &other) = delete;

    /**
     * Destructor
     */
    virtual ~KDSoapServerCustomVerbRequestInterface();

    /**
     * Process a request made with a custom HTTP verb
     * @param requestType HTTP verb other than GET and POST
     * @param requestData is the content of the request
     * @param httpHeaders the map of http headers (keys have been lowercased since they are case insensitive)
     * @param customAnswer allow to send back the answer to the client if the request has been handled
     * @return true if the request has been handled and if customAnswer is valid and will be sent back to the client.
     */
    virtual bool processCustomVerbRequest(const QByteArray &requestType, const QByteArray &requestData,
                                          const QMap<QByteArray, QByteArray> &httpHeaders, QByteArray &customAnswer);

private:
    friend class KDSoapServerSocket;
    class Private;
    Private *const d;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(KDSoapServerCustomVerbRequestInterface, "com.kdab.KDSoap.ServerCustomVerbRequestInterface/1.0")
QT_END_NAMESPACE

#endif /* KDSOAPSERVERCUSTOMVERBREQUESTINTERFACE_H */
