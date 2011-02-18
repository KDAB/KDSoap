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
    void writeElementContents(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValue& element, KDSoapMessage::Use use) const;
    void writeChildren(KDSoapNamespacePrefixes& namespacePrefixes, QXmlStreamWriter& writer, const KDSoapValueList& args, KDSoapMessage::Use use) const;
    void writeAttributes(QXmlStreamWriter& writer, const QList<KDSoapValue>& attributes) const;

private:
    QString m_messageNamespace;
};

#endif // KDSOAPMESSAGEWRITER_P_H
