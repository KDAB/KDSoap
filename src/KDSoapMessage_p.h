#ifndef KDSOAPMESSAGE_P_H
#define KDSOAPMESSAGE_P_H

#include "KDSoapValue.h"

// TODO: can be moved back to KDSoapMessage.cpp I think

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapValueList args;
    bool isFault;
};

#endif // KDSOAPMESSAGE_P_H
