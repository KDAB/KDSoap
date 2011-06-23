#ifndef KDSOAPNAMESPACEMANAGER_H
#define KDSOAPNAMESPACEMANAGER_H

#include "KDSoapGlobal.h"
#include <QString>

/**
 * Repository of namespaces
 */
class KDSOAPCLIENT_EXPORT KDSoapNamespaceManager
{
public:
    static QString xmlSchema1999();
    static QString xmlSchema2001();
    static QString xmlSchemaInstance1999();
    static QString xmlSchemaInstance2001();
    static QString soapEnvelope();
    static QString soapEncoding();

private: // TODO instanciate to handle custom namespaces per clientinterface
    KDSoapNamespaceManager();
};

#endif // KDSOAPNAMESPACEMANAGER_H
