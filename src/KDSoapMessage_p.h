#ifndef KDSOAPMESSAGE_P_H
#define KDSOAPMESSAGE_P_H

#include "KDSoapValue.h"

class KDSoapMessageData : public QSharedData
{
public:
    KDSoapValueList args;
};

#endif // KDSOAPMESSAGE_P_H
