#include "KDSoapDelayedResponseHandle.h"
#include "KDSoapServerSocket_p.h"
#include <QSharedData>

class KDSoapDelayedResponseHandleData : public QSharedData {
public:
    KDSoapDelayedResponseHandleData(KDSoapServerSocket* s)
        : socket(s)
    {}
    KDSoapServerSocket* socket;
};

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle() : data(new KDSoapDelayedResponseHandleData(0))
{
}

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle(const KDSoapDelayedResponseHandle &rhs) : data(rhs.data)
{
}

KDSoapDelayedResponseHandle &KDSoapDelayedResponseHandle::operator=(const KDSoapDelayedResponseHandle &rhs)
{
    if (this != &rhs)
        data.operator=(rhs.data);
    return *this;
}

KDSoapDelayedResponseHandle::~KDSoapDelayedResponseHandle()
{
}

KDSoapDelayedResponseHandle::KDSoapDelayedResponseHandle(KDSoapServerSocket* socket)
    : data(new KDSoapDelayedResponseHandleData(socket))
{
    socket->setResponseDelayed();
}

KDSoapServerSocket * KDSoapDelayedResponseHandle::serverSocket() const
{
    return data->socket;
}
