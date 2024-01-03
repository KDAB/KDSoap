/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef KDSOAPCLIENTTHREAD_P_H
#define KDSOAPCLIENTTHREAD_P_H

#include "KDSoapAuthentication.h"
#include "KDSoapMessage.h"
#include <QtCore/QMutex>
#include <QtCore/QQueue>
#include <QtCore/QSemaphore>
#include <QtCore/QThread>
#include <QtCore/QWaitCondition>
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

// clazy:excludeall=ctor-missing-parent-argument
class KDSoapThreadTask : public QObject
{
    Q_OBJECT
public:
    explicit KDSoapThreadTask(KDSoapThreadTaskData *data) // clazy:exclude=ctor-missing-parent-argument
        : m_data(data)
    {
    }

    void process(QNetworkAccessManager &accessManager);
    void slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator);

signals:
    void taskDone();

private Q_SLOTS:
    void slotFinished(KDSoapPendingCallWatcher *watcher);

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
