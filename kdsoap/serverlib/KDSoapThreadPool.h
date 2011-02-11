#ifndef KDSOAPTHREADPOOL_H
#define KDSOAPTHREADPOOL_H

class KDSoapSocketPool;

/**
 *
 */
class KDSoapThreadPool : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs an item delegate with the given \p parent.
     */
    KDSoapThreadPool(QObject* parent = 0);

    // TODO setMaxThreads

    void handleIncomingConnection(int socketDescriptor);

private:
    KDSoapSocketPool* m_mainThreadSocketPool;
};

#endif // KDSOAPTHREADPOOL_H
