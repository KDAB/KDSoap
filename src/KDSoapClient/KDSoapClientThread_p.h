/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/

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
    KDSoapThreadTaskData(KDSoapClientInterface *iface, const QString &method, const KDSoapMessage &message, const QString &action,
                         const KDSoapHeaders &headers)
        : m_iface(iface)
        , m_method(method)
        , m_message(message)
        , m_action(action)
        , m_headers(headers)
    {
    }

    void waitForCompletion()
    {
        m_semaphore.acquire();
    }
    KDSoapMessage response() const
    {
        return m_response;
    }
    KDSoapHeaders responseHeaders() const
    {
        return m_responseHeaders;
    }

    KDSoapClientInterface *m_iface; // used by KDSoapThreadTask::process()
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
    explicit KDSoapThreadTask(KDSoapThreadTaskData *data)
        : m_data(data)
    {
    }

    void process(QNetworkAccessManager &accessManager);

signals:
    void taskDone();

private Q_SLOTS:
    void slotFinished(KDSoapPendingCallWatcher *watcher);
    void slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

private:
    KDSoapThreadTaskData *m_data;
};

class KDSoapClientThread : public QThread
{
    Q_OBJECT
public:
    explicit KDSoapClientThread(QObject *parent = nullptr);

    void enqueue(KDSoapThreadTaskData *taskData);

    void stop();

protected:
    virtual void run() override;

private:
    QMutex m_mutex;
    QQueue<KDSoapThreadTaskData *> m_queue;
    QWaitCondition m_queueNotEmpty;
    bool m_stopThread;
};

#endif // KDSOAPCLIENTTHREAD_P_H
