#ifndef KDSOAPDELAYEDRESPONSEHANDLE_H
#define KDSOAPDELAYEDRESPONSEHANDLE_H

#include "KDSoapServerGlobal.h"
#include <QSharedDataPointer>

class KDSoapDelayedResponseHandleData;
class KDSoapServerSocket;

/**
 * The delayed-response-handle is an opaque data type representing
 * a delayed response. When a server object wants to implement a SOAP method
 * call using an asynchronous operation, it can call prepareDelayedResponse(), store
 * the handle, and use the handle later on in order to send the delayed response.
 */
class KDSOAPSERVER_EXPORT KDSoapDelayedResponseHandle
{
public:
    /**
     * Constructs a null handle.
     */
    KDSoapDelayedResponseHandle();
    KDSoapDelayedResponseHandle(const KDSoapDelayedResponseHandle &);
    KDSoapDelayedResponseHandle &operator=(const KDSoapDelayedResponseHandle &);
    ~KDSoapDelayedResponseHandle();

private:
    friend class KDSoapServerObjectInterface;
    explicit KDSoapDelayedResponseHandle(KDSoapServerSocket* socket);
    KDSoapServerSocket* serverSocket() const;
    QSharedDataPointer<KDSoapDelayedResponseHandleData> data;
};

#endif // KDSOAPDELAYEDRESPONSEHANDLE_H
