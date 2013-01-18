/****************************************************************************
** Copyright (C) 2010-2013 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#ifndef KDSOAPPENDINGCALL_H
#define KDSOAPPENDINGCALL_H

#include <QtCore/QExplicitlySharedDataPointer>
#include "KDSoapMessage.h"
QT_BEGIN_NAMESPACE
class QNetworkReply;
class QBuffer;
QT_END_NAMESPACE
class KDSoapPendingCallWatcher;

/**
 * The KDSoapPendingCall class refers to one pending asynchronous call
 *
 * A KDSoapPendingCall object is a reference to a SOAP method call that was sent over
 * without waiting for a reply.
 * KDSoapPendingCall is an opaque type, meant to be used as a handle for a pending reply.
 *
 * The KDSoapPendingCallWatcher class allows one to connect to a signal that will
 * indicate when the reply has arrived or if an error occurred.
 *
 * \note If you create a copy of a KDSoapPendingCall object, all information will
 * be shared among the many copies. Therefore, KDSoapPendingCall is an
 * explicitly-shared object and does not provide a method of detaching the copies
 * (since they refer to the same pending call)
 *
 * \see KDSoapClientInterface::asyncCall()
 */
class KDSOAP_EXPORT KDSoapPendingCall
{
public:
    /**
     * Creates a copy of the \p other pending asynchronous call.
     * Note that both objects will refer to the same pending call.
     */
    KDSoapPendingCall(const KDSoapPendingCall &other);

    /**
     * Destroys this copy of the KDSoapPendingCall object.
     * \warning If this copy is also the last copy of a pending asynchronous call,
     * the call will be canceled and no further notifications will be received.
     */
    ~KDSoapPendingCall();

    /**
     * Creates a copy of the \p other pending asynchronous call and drops
     * the reference to the previously-referenced call. Note that both objects
     * will refer to the same pending call after this function.
     *
     * \warning If this object contained the last reference of a pending asynchronous
     * call, the call will be canceled and no further notifications will be
     * received.
     */
    KDSoapPendingCall &operator=(const KDSoapPendingCall &other);

    /**
     * Returns the response message sent by the server.
     * Could either be a fault (see KDSoapMessage::isFault) or the actual response arguments.
     */
    KDSoapMessage returnMessage() const;

    /**
     * Helper method for the simple case where a single argument is returned:
     * Returns the value of that single argument.
     */
    QVariant returnValue() const;

    /**
     * Returns the response headers sent by the server.
     */
    KDSoapHeaders returnHeaders() const;

    /**
     * Returns \c true if the pending call has finished processing and the reply has been received.
     *
     * \note This function only changes state if an external event happens,
     * which in general only happens if you return to the event loop execution.
     *
     * You generally do not need to use this: use KDSoapPendingCallWatcher to be notified
     * of the call completion.
     *
     * \warning This method requires Qt 4.6 to work properly. Do not call isFinished when
     * KDSoap was compiled against 4.4 or 4.5, it will always return \c false.
     */
    bool isFinished() const;

private:
    friend class KDSoapClientInterface;
    friend class KDSoapThreadTask;
    KDSoapPendingCall(QNetworkReply* reply, QBuffer* buffer);

    friend class KDSoapPendingCallWatcher; // for connecting to d->reply

    class Private;
    QExplicitlySharedDataPointer<Private> d;
};

#endif // KDSOAPPENDINGCALL_H
