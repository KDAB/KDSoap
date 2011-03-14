#include "KDSoapNamespacePrefixes_p.h"
#include "KDSoapNamespaceManager.h"

void KDSoapNamespacePrefixes::writeStandardNamespaces(QXmlStreamWriter& writer)
{
    writeNamespace(writer, KDSoapNamespaceManager::soapEnvelope(), QLatin1String("soap"));
    writeNamespace(writer, KDSoapNamespaceManager::soapEncoding(), QLatin1String("soap-enc"));
    writeNamespace(writer, KDSoapNamespaceManager::xmlSchema1999(), QLatin1String("xsd"));
    writeNamespace(writer, KDSoapNamespaceManager::xmlSchemaInstance1999(), QLatin1String("xsi"));

    // Also insert known variants
    insert(KDSoapNamespaceManager::xmlSchema2001(), QString::fromLatin1("xsd"));
    insert(KDSoapNamespaceManager::xmlSchemaInstance2001(), QString::fromLatin1("xsi"));
}
