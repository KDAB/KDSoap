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
#ifndef KDSOAPSERVERCUSTOMVERBREQUESTINTERFACE_H
#define KDSOAPSERVERCUSTOMVERBREQUESTINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QObject>
#include <QtCore/QMap>
class KDSoapAuthentication;
class KDSoapServerSocket;

/**
 * Additional interface for handling custom verb request.
 *
 * In addition to deriving from KDSoapServerObjectInterface, you can derive from
 * KDSoapServerCustomVerbRequestInterface in order to handle custom verb HTML requests.
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
    Private* const d;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(KDSoapServerCustomVerbRequestInterface,
                    "com.kdab.KDSoap.ServerCustomVerbRequestInterface/1.0")
QT_END_NAMESPACE

#endif /* KDSOAPSERVERCUSTOMVERBREQUESTINTERFACE_H */
