#ifndef KDSOAPMESSAGE_P_H
#define KDSOAPMESSAGE_P_H

#include "KDSoapValue.h"
#include "KDSoapMessage.h"

// This was in a separate _p.h file so that KDSoapClientInterface could use this.
// TODO: clean up

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapMessageData()
        : use(KDSoapMessage::LiteralUse), isFault(false)
    {}

    KDSoapMessage::Use use;
    bool isFault;
};

#endif // KDSOAPMESSAGE_P_H
