#ifndef KDSOAPCLIENTTHREAD_P_H
#define KDSOAPCLIENTTHREAD_P_H

#include "KDSoapMessage.h"
#include "KDSoapAuthentication.h"
#include <QWaitCondition>
#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QSemaphore>
#include <QNetworkAccessManager>

class KDSoapPendingCallWatcher;
class KDSoapClientInterface;
class QEventLoop;

class KDSoapThreadTaskData
{
public:
    KDSoapThreadTaskData(KDSoapClientInterface* iface, const QString& method, const KDSoapMessage &message, const QString& action)
        : m_iface(iface), m_method(method), m_message(message), m_action(action) {}

    void waitForCompletion() { m_semaphore.acquire(); }
    KDSoapMessage returnArguments() const { return m_returnArguments; }

    KDSoapClientInterface* m_iface; // used by KDSoapThreadTask::process()
    KDSoapAuthentication m_authentication;
    QString m_method;
    KDSoapMessage m_message;
    QString m_action;
    QSemaphore m_semaphore;
    KDSoapMessage m_returnArguments;
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
