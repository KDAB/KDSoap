#ifndef KSOAPMESSAGEREADER_P_H
#define KSOAPMESSAGEREADER_P_H

#include "KDSoapMessage.h"

class KDSOAP_EXPORT KDSoapMessageReader
{
public:
    enum XmlError {
        NoError=0,
        ParseError,
        PrematureEndOfDocumentError
    };

    KDSoapMessageReader();

    XmlError xmlToMessage(const QByteArray& data, KDSoapMessage* pParsedMessage, QString* pMessageNamespace, KDSoapHeaders* pRequestHeaders) const;
};

#endif
