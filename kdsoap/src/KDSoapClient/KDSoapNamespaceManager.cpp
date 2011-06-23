#include "KDSoapNamespaceManager.h"

KDSoapNamespaceManager::KDSoapNamespaceManager()
{
}

QString KDSoapNamespaceManager::xmlSchema1999()
{
    static QString s = QString::fromLatin1("http://www.w3.org/1999/XMLSchema");
    return s;
}

QString KDSoapNamespaceManager::xmlSchema2001()
{
    static QString s = QString::fromLatin1("http://www.w3.org/2001/XMLSchema");
    return s;
}

QString KDSoapNamespaceManager::xmlSchemaInstance1999()
{
    static QString s = QString::fromLatin1("http://www.w3.org/1999/XMLSchema-instance");
    return s;
}

QString KDSoapNamespaceManager::xmlSchemaInstance2001()
{
    static QString s = QString::fromLatin1("http://www.w3.org/2001/XMLSchema-instance");
    return s;
}

QString KDSoapNamespaceManager::soapEnvelope()
{
    static QString s = QString::fromLatin1("http://schemas.xmlsoap.org/soap/envelope/");
    return s;
}

QString KDSoapNamespaceManager::soapEncoding()
{
    static QString s = QString::fromLatin1("http://schemas.xmlsoap.org/soap/encoding/");
    return s;
}
