/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPPENDINGCALLWATCHER_H
#define KDSOAPPENDINGCALLWATCHER_H

#include "KDSoapPendingCall.h"
#include <QtCore/QObject>

/**
 * The KDSoapPendingCallWatcher class provides a convenient way for waiting for
 * asynchronous replies.
 *
 * KDSoapPendingCallWatcher provides the finished() signal that will be emitted
 * when a reply arrives.
 *
 * It is usually used like the following example:
 * \code
 *  KDSoapPendingCall pendingCall = client.asyncCall(QLatin1String("MethodName"), message);
 *  KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
 *
 *  QObject::connect(watcher, &KDSoapPendingCallWatcher::finished,
 *                   this, &MyClass::slotFinished);
 * \endcode
 *
 * \note It is not necessary to keep the original KDSoapPendingCall object
 * around since KDSoapPendingCallWatcher inherits from that class too.
 */
class KDSOAP_EXPORT KDSoapPendingCallWatcher : public QObject, public KDSoapPendingCall
{
    Q_OBJECT
public:
    /**
     * Creates a KDSoapPendingCallWatcher object to watch for replies on the
     * asynchronous pending call \p call and sets this object's parent to
     * \p parent.
     */
    explicit KDSoapPendingCallWatcher(const KDSoapPendingCall &call, QObject *parent = nullptr);
    /**
     * Destroys this object. If this KDSoapPendingCallWatcher object was the last reference to the unfinished pending call, the call will be canceled.
     */
    ~KDSoapPendingCallWatcher();

Q_SIGNALS:
    /**
     * This signal is emitted when the pending call has finished and its reply
     * is available. The \p self parameter is a pointer to the object itself,
     * passed for convenience so that the slot can access the properties and
     * determine the contents of the reply.
     */
    void finished(KDSoapPendingCallWatcher *self);

private:
    friend class KDSoapPendingCallPrivate;

    class Private;
    Private *const d;
};

#endif // KDSOAPPENDINGCALLWATCHER_H
