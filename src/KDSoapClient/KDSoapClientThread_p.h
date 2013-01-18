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

#ifndef KDSOAPCLIENTTHREAD_P_H
#define KDSOAPCLIENTTHREAD_P_H

#include "KDSoapMessage.h"
#include "KDSoapAuthentication.h"
#include <QtCore/QWaitCondition>
#include <QtCore/QQueue>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QSemaphore>
#include <QtNetwork/QNetworkAccessManager>

class KDSoapPendingCallWatcher;
class KDSoapClientInterface;
QT_BEGIN_NAMESPACE
class QEventLoop;
QT_END_NAMESPACE

class KDSoapThreadTaskData
{
public:
    KDSoapThreadTaskData(KDSoapClientInterface* iface, const QString& method, const KDSoapMessage &message, const QString& action, const KDSoapHeaders& headers)
        : m_iface(iface), m_method(method), m_message(message), m_action(action), m_headers(headers) {}

    void waitForCompletion() { m_semaphore.acquire(); }
    KDSoapMessage response() const { return m_response; }
    KDSoapHeaders responseHeaders() const { return m_responseHeaders; }

    KDSoapClientInterface* m_iface; // used by KDSoapThreadTask::process()
    KDSoapAuthentication m_authentication;
    QString m_method;
    KDSoapMessage m_message;
    QString m_action;
    QSemaphore m_semaphore;
    KDSoapMessage m_response;
    KDSoapHeaders m_responseHeaders;
    KDSoapHeaders m_headers;
};

class KDSoapThreadTask : public QObject
{
    Q_OBJECT
public:
    KDSoapThreadTask(KDSoapThreadTaskData* data)
        : m_data(data) {}

    void process(QNetworkAccessManager& accessManager);

signals:
    void taskDone();

private Q_SLOTS:
    void slotFinished(KDSoapPendingCallWatcher* watcher);
    void slotAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);

private:
    KDSoapThreadTaskData* m_data;
};

class KDSoapClientThread : public QThread
{
    Q_OBJECT
public:
    explicit KDSoapClientThread(QObject *parent = 0);

    void enqueue(KDSoapThreadTaskData* taskData);

    void stop();

protected:
    virtual void run();

private:
    QMutex m_mutex;
    QQueue<KDSoapThreadTaskData*> m_queue;
    QWaitCondition m_queueNotEmpty;
    bool m_stopThread;
};

#endif // KDSOAPCLIENTTHREAD_P_H
