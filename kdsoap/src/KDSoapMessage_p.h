#ifndef KDSOAPMESSAGE_P_H
#define KDSOAPMESSAGE_P_H

#include "KDSoapValue.h"
#include "KDSoapMessage.h"

// This is in a separate _p.h file so that KDSoapClientInterface can use this.

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapMessageData()
        : args(), use(KDSoapMessage::LiteralUse), isFault(false)
    {}

    KDSoapValueList args;
    KDSoapMessage::Use use;
    bool isFault;
};

#endif // KDSOAPMESSAGE_P_H
