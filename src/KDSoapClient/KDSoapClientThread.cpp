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

#include "KDSoapClientThread_p.h"
#include <QDebug>
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapPendingCall.h"
#include <QNetworkRequest>
#include <QNetworkProxy>
#include <QBuffer>
#include <QEventLoop>
#include <QAuthenticator>

KDSoapClientThread::KDSoapClientThread(QObject *parent) :
    QThread(parent), m_stopThread(false)
{
}

// Called by the main thread
void KDSoapClientThread::enqueue(KDSoapThreadTaskData* taskData)
{
    QMutexLocker locker(&m_mutex);
    m_queue.append(taskData);
    m_queueNotEmpty.wakeOne();
}

void KDSoapClientThread::run()
{
    QNetworkAccessManager accessManager;
    QEventLoop eventLoop;

    while ( true ) {
        QMutexLocker locker( &m_mutex );
        if (!m_stopThread && m_queue.isEmpty()) {
            m_queueNotEmpty.wait( &m_mutex );
        }
        if (m_stopThread) {
            break;
        }
        KDSoapThreadTaskData* taskData = m_queue.dequeue();
        locker.unlock();

        KDSoapThreadTask task(taskData); // must be created here, so that it's in the right thread
        connect(&task, SIGNAL(taskDone()), &eventLoop, SLOT(quit()));
        connect(&accessManager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
                &task, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
        task.process(accessManager);

        // Process events until the task tells us the handling of that task is finished
        eventLoop.exec();
    }
}

void KDSoapThreadTask::process(QNetworkAccessManager& accessManager)
{
    // Can't use m_iface->asyncCall, it would use the accessmanager from the main thread
    //KDSoapPendingCall pendingCall = m_iface->asyncCall(m_method, m_message, m_action);

    //Headers should be always qualified
    for (KDSoapHeaders::Iterator it = m_data->m_headers.begin(); it != m_data->m_headers.end(); ++it) {
        it->setQualified(true);
    }

#if QT_VERSION >= 0x040700
    QNetworkCookieJar* jar = m_data->m_iface->d->m_accessManager.cookieJar();
    // Qt-4.6: this aborts in setParent(this) because the jar is from another thread
    // Qt-4.7: it's from a different thread, so this won't change the parent object
    accessManager.setCookieJar(jar);
#endif

    accessManager.setProxy( m_data->m_iface->d->m_accessManager.proxy() );

    QBuffer* buffer = m_data->m_iface->d->prepareRequestBuffer(m_data->m_method, m_data->m_message, m_data->m_headers);
    QNetworkRequest request = m_data->m_iface->d->prepareRequest(m_data->m_method, m_data->m_action);
    QNetworkReply* reply = accessManager.post(request, buffer);
    m_data->m_iface->d->setupReply(reply);
    KDSoapPendingCall pendingCall(reply, buffer);

    KDSoapPendingCallWatcher *watcher = new KDSoapPendingCallWatcher(pendingCall, this);
    connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
            this, SLOT(slotFinished(KDSoapPendingCallWatcher*)));
}

void KDSoapThreadTask::slotFinished(KDSoapPendingCallWatcher* watcher)
{
    m_data->m_response = watcher->returnMessage();
    m_data->m_responseHeaders = watcher->returnHeaders();
    m_data->m_semaphore.release();
    // Helgrind bug: says this races with main thread. Looks like it's confused by QSharedDataPointer
    //qDebug() << m_data->m_returnArguments.value();
    watcher->deleteLater();

    emit taskDone();
}

void KDSoapClientThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stopThread = true;
    m_queueNotEmpty.wakeAll();
}

void KDSoapThreadTask::slotAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    m_data->m_authentication.handleAuthenticationRequired(reply, authenticator);
}
