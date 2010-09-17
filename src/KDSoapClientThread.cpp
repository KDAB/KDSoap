#include "KDSoapClientThread_p.h"
#include <QDebug>
#include "KDSoapPendingCallWatcher.h"
#include "KDSoapClientInterface.h"
#include "KDSoapClientInterface_p.h"
#include "KDSoapPendingCall.h"
#include <QNetworkRequest>
#include <QBuffer>
#include <QEventLoop>
#include <QAuthenticator>

KDSoapClientThread::KDSoapClientThread(QObject *parent) :
    QThread(parent), m_stopThread(false)
{
}

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
        m_mutex.lock();
        if (!m_stopThread && m_queue.isEmpty()) {
            m_queueNotEmpty.wait( &m_mutex );
        }
        if (m_stopThread) {
            m_mutex.unlock();
            break;
        }
        KDSoapThreadTaskData* taskData = m_queue.dequeue();
        m_mutex.unlock();

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
    m_data->m_returnArguments = watcher->returnMessage();
    m_data->m_semaphore.release();
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
