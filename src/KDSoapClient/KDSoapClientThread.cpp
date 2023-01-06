/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapClientThread_p.h"
#include "KDSoapPendingCall.h"
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapPendingCall_p.h"
#include <QAuthenticator>
#include <QBuffer>
#include <QDebug>
#include <QEventLoop>
#include <QNetworkProxy>
#include <QNetworkRequest>

KDSoapClientThread::KDSoapClientThread(QObject *parent)
    : QThread(parent)
    , m_stopThread(false)
{
}

// Called by the main thread
void KDSoapClientThread::enqueue(KDSoapThreadTaskData *taskData)
{
    // On hindsight, it would have been simpler to use signal/slots
    // to communicate with secondary thread (turning this class
    // into a simple QObject with a QThread internally)

    QMutexLocker locker(&m_mutex);
    m_queue.append(taskData);
    m_queueNotEmpty.wakeOne();
}

void KDSoapClientThread::run()
{
    QNetworkAccessManager accessManager;
    // Use own QEventLoop so its slot quit() is executed in this thread
    // (using QThread::exec/quit would try to call QThread::quit() in main thread,
    //  which is blocked on semaphore)
    QEventLoop eventLoop;

    while (true) {
        QMutexLocker locker(&m_mutex);
        while (!m_stopThread && m_queue.isEmpty()) {
            m_queueNotEmpty.wait(&m_mutex);
        }
        if (m_stopThread) {
            break;
        }
        KDSoapThreadTaskData *taskData = m_queue.dequeue();
        locker.unlock();

        KDSoapThreadTask task(taskData); // must be created here, so that it's in the right thread
        connect(&task, &KDSoapThreadTask::taskDone, &eventLoop, &QEventLoop::quit);
        connect(&accessManager, &QNetworkAccessManager::authenticationRequired, &task, &KDSoapThreadTask::slotAuthenticationRequired);
        task.process(accessManager);

        // Process events until the task tells us the handling of that task is finished
        eventLoop.exec();
    }
}

void KDSoapThreadTask::process(QNetworkAccessManager &accessManager)
{
    // Can't use m_iface->asyncCall, it would use the accessmanager from the main thread
    // KDSoapPendingCall pendingCall = m_iface->asyncCall(m_method, m_message, m_action);

    // Headers should be always qualified
    for (KDSoapMessage &header : m_data->m_headers) {
        header.setQualified(true);
    }

    QNetworkCookieJar *jar = m_data->m_iface->d->accessManager()->cookieJar();
    accessManager.setCookieJar(jar);

    accessManager.setProxy(m_data->m_iface->d->accessManager()->proxy());

    QBuffer *buffer = m_data->m_iface->d->prepareRequestBuffer(m_data->m_method,
                                                               m_data->m_message,
                                                               m_data->m_action,
                                                               m_data->m_headers);
    QNetworkRequest request = m_data->m_iface->d->prepareRequest(m_data->m_method, m_data->m_action);
    QNetworkReply *reply = accessManager.post(request, buffer);
    m_data->m_iface->d->setupReply(reply);
    maybeDebugRequest(buffer->data(), reply->request(), reply);
    KDSoapPendingCall pendingCall(reply, buffer);
    pendingCall.d->soapVersion = m_data->m_iface->d->m_version;

    KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
    connect(watcher, &KDSoapPendingCallWatcher::finished, this, &KDSoapThreadTask::slotFinished);
}

void KDSoapThreadTask::slotFinished(KDSoapPendingCallWatcher *watcher)
{
    m_data->m_response = watcher->returnMessage();
    m_data->m_responseHeaders = watcher->returnHeaders();
    m_data->m_semaphore.release();
    // Helgrind bug: says this races with main thread. Looks like it's confused by QSharedDataPointer
    // qDebug() << m_data->m_returnArguments.value();
    watcher->deleteLater();

    emit taskDone();
}

void KDSoapClientThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopThread = true;
    m_queueNotEmpty.wakeAll();
}

void KDSoapThreadTask::slotAuthenticationRequired(QNetworkReply *reply, QAuthenticator *authenticator)
{
    m_data->m_authentication.handleAuthenticationRequired(reply, authenticator);
}
