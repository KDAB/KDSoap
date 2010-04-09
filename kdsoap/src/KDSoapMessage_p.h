#ifndef KDSOAPMESSAGE_P_H
#define KDSOAPMESSAGE_P_H

#include "KDSoapValue.h"
#include "KDSoapMessage.h"

// TODO: can be moved back to KDSoapMessage.cpp

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapMessageData()
        : isFault(false)
    {}

    KDSoapValueList args;
    KDSoapMessage::Use use;
    bool isFault;
};

#endif // KDSOAPMESSAGE_P_H
