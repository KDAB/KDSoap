#ifndef KDSOAPMESSAGEWRITER_P_H
#define KDSOAPMESSAGEWRITER_P_H

#include "KDSoapMessage.h"
#include <QXmlStreamWriter>
#include <QByteArray>
#include <QString>
#include <QMap>
class KDSoapMessage;
class KDSoapHeaders;
class KDSoapNamespacePrefixes;
class KDSoapValue;
class KDSoapValueList;

class KDSOAP_EXPORT KDSoapMessageWriter
{
public:
    KDSoapMessageWriter();

    void setMessageNamespace(const QString& ns);

    QByteArray messageToXml(const KDSoapMessage& message, const QString& method,
                            const KDSoapHeaders& headers,
                            const QMap<QString, KDSoapMessage>& persistentHeaders) const;

private:
    QString m_messageNamespace;
};

#endif // KDSOAPMESSAGEWRITER_P_H
