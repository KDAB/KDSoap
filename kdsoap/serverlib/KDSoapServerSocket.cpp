#include "KDSoapServerSocket_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerObjectInterface.h"
#include <KDSoapMessage.h>
#include <KDSoapNamespaceManager.h>
#include <QBuffer>
#include <QMetaMethod>
#include <QVarLengthArray>

KDSoapServerSocket::KDSoapServerSocket(KDSoapSocketList* owner, QObject* serverObject)
    : QTcpSocket(),
      m_owner(owner),
      m_serverObject(serverObject)
{
    connect(this, SIGNAL(readyRead()),
            this, SLOT(slotReadyRead()));
}

KDSoapServerSocket::~KDSoapServerSocket()
{
    m_owner->socketDeleted(this);
}


typedef QMap<QByteArray, QByteArray> HeadersMap;
static HeadersMap parseHeaders(const QByteArray& headerData)
{
    HeadersMap headersMap;
    QBuffer sourceBuffer;
    sourceBuffer.setData(headerData);
    sourceBuffer.open(QIODevice::ReadOnly);
    // The first line is special, it's the GET or POST line
    const QList<QByteArray> firstLine = sourceBuffer.readLine().split(' ');
    if (firstLine.count() < 3) {
        qDebug() << "Malformed HTTP request:" << firstLine;
        return headersMap;
    }
    const QByteArray request = firstLine.at(0);
    const QByteArray path = firstLine.at(1);
    const QByteArray httpVersion = firstLine.at(2);
    if (request != "GET" && request != "POST") {
        qDebug() << "Unknown HTTP request:" << firstLine;
        return headersMap;
    }
    headersMap.insert("_path", path);
    headersMap.insert("_httpVersion", httpVersion);

    while (!sourceBuffer.atEnd()) {
        const QByteArray line = sourceBuffer.readLine();
        const int pos = line.indexOf(':');
        if (pos == -1)
            qDebug() << "Malformed HTTP header:" << line;
        const QByteArray header = line.left(pos);
        const QByteArray value = line.mid(pos+1).trimmed(); // remove space before and \r\n after
        //qDebug() << "HEADER" << header << "VALUE" << value;
        headersMap.insert(header, value);
    }
    return headersMap;
}

static bool splitHeadersAndData(const QByteArray& request, QByteArray& header, QByteArray& data)
{
    const int sep = request.indexOf("\r\n\r\n");
    if (sep <= 0)
        return false;
    header = request.left(sep);
    data = request.mid(sep + 4);
    return true;
}

static int kdSoapNameToTypeId(const char *name)
{
    int id = static_cast<int>( QVariant::nameToType(name) );
    if (id == QVariant::UserType)
        id = QMetaType::type(name);
    return id;
}

// Reverse operation from variantToXmlType in KDSoapClientInterface
static QByteArray xmlTypeToVariant(const QString& xmlType)
{
    static const struct {
        const char* xml; // xsd: prefix assumed
        const char* qType;
    } s_types[] = {
        { "string", "QString" }, // or QUrl
        { "base64Binary", "QByteArray" },
        { "int", "int" }, // or long, or uint, or longlong
        { "unsignedInt", "qulonglong" },
        { "boolean", "bool" },
        { "float", "float" },
        { "double", "double" },
        { "time", "QTime" },
        { "date", "QDate" },
        { "dateTime", "QDateTime" }
    };
    // Speed: could be sorted and then we could use qBinaryFind
    static const int s_numTypes = sizeof(s_types) / sizeof(*s_types);
    for (int i = 0; i < s_numTypes; ++i) {
        if (xmlType == QLatin1String(s_types[i].xml)) {
            return s_types[i].qType;
        }
    }
    qDebug() << QString::fromLatin1("xmlTypeToVariant: XML type %1 is not supported in "
                                    "KDSoap, see the documentation").arg(xmlType);
    return QByteArray();
}

#if 0
// calculates the metatypes for the method
// the slot must have the parameters in the following form:
//  - zero or more value or const-ref parameters of any kind
//  - zero or more non-const ref parameters
// No parameter may be a template.
// this function returns -1 if the parameters don't match the above form
// this function returns the number of *input* parameters
// this function does not check the return type, so metaTypes[0] is always 0 and always present
// metaTypes.count() >= retval + 1 in all cases
//
// sig must be the normalised signature for the method
static int kdSoapParametersForMethod(const QMetaMethod &mm, QList<int>& metaTypes)
{
    //QDBusMetaTypeId::init();

    QList<QByteArray> parameterTypes = mm.parameterTypes();
    metaTypes.clear();

    metaTypes.append(0);        // return type
    int inputCount = 0;
    QList<QByteArray>::ConstIterator it = parameterTypes.constBegin();
    QList<QByteArray>::ConstIterator end = parameterTypes.constEnd();
    for ( ; it != end; ++it) {
        const QByteArray &type = *it;
        if (type.endsWith('*')) {
            //qWarning("Could not parse the method '%s'", mm.signature());
            // pointer?
            return -1;
        }

        if (type.endsWith('&')) {
            QByteArray basictype = type;
            basictype.truncate(type.length() - 1);

            int id = kdSoapNameToTypeId(basictype.constData());
            if (id == 0) {
                //qWarning("Could not parse the method '%s'", mm.signature());
                // invalid type in method parameter list
                return -1;
            }// else if (QDBusMetaType::typeToSignature(id) == 0)
            //    return -1;

            metaTypes.append( id );
            continue;
        }

        int id = kdSoapNameToTypeId(type.constData());
        if (id == 0) {
            //qWarning("Could not parse the method '%s'", mm.signature());
            // invalid type in method parameter list
            return -1;
        }

        //if (QDBusMetaType::typeToSignature(id) == 0)
        //    return -1;

        metaTypes.append(id);
        ++inputCount;
    }

    return inputCount;
}
#endif

static int findSlot(const QMetaObject *mo, const QByteArray &name,
                    const QList<QByteArray>& qtTypes, int* returnMetaType)
{
    for (int idx = mo->methodCount() - 1 ; idx >= QObject::staticMetaObject.methodCount(); --idx) {
        QMetaMethod mm = mo->method(idx);

        // check access:
        if (mm.access() != QMetaMethod::Public)
            continue;

        // check type:
        if (mm.methodType() != QMetaMethod::Slot)
            continue;

        // check name:
        QByteArray slotname = mm.signature();
        int paren = slotname.indexOf('(');
        if (paren != name.length() || !slotname.startsWith(name))
            continue;

        // check argument types:
        if (mm.parameterTypes() != qtTypes) {
            continue;
        }

        const int returnType = kdSoapNameToTypeId(mm.typeName());

#if 0
        // Fill in the metaTypes array for this slot
        QList<int> foundMetaTypes;
        int inputCount = kdSoapParametersForMethod(mm, foundMetaTypes);
        if (inputCount == -1)
            continue;           // problem parsing

        if (foundMetaTypes != metaTypes)
            continue;
#endif

        *returnMetaType = returnType;

        // if we got here, this slot matched
        return idx;
    }

    // no slot matched
    return -1;
}

#if 0
// This would be if we had the full signature, but we don't
int QDBusConnectionPrivate::findSlot(QObject* obj, const QByteArray &normalizedName,
                                     QList<int> &params)
{
    int midx = obj->metaObject()->indexOfMethod(normalizedName);
    if (midx == -1)
        return -1;

    int inputCount = kdSoapParametersForMethod(obj->metaObject()->method(midx), params);
    if ( inputCount == -1 || inputCount + 1 != params.count() )
        return -1;              // failed to parse or invalid arguments or output arguments

    return midx;
}
#endif

void KDSoapServerSocket::slotReadyRead()
{
    qDebug() << "slotReadyRead!";
    const QByteArray request = this->readAll(); // ## TODO what if it's not all available?

    //qDebug() << "KDSoapServerSocket: request:" << request;

    const bool splitOK = splitHeadersAndData(request, m_receivedHeaders, m_receivedData);
    Q_ASSERT(splitOK);
    Q_UNUSED(splitOK); // To avoid a warning if Q_ASSERT doesn't expand to anything.
    m_headers = parseHeaders(m_receivedHeaders);

    qDebug() << "headers received:" << m_receivedHeaders;
    qDebug() << m_headers;
    qDebug() << "data received:" << m_receivedData;

    KDSoapMessage requestMsg;
    requestMsg.parseSoapXml(m_receivedData);
    KDSoapMessage replyMsg;
    if (requestMsg.isFault()) {
        // TODO fault handling, in replyMsg
    } else {
        // Call method on m_serverObject
        KDSoapServerObjectInterface* serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(m_serverObject);

        serverObjectInterface->resetFault();
        // TODO serverObjectInterface->setHeaders(headers);

        const QByteArray method = requestMsg.name().toLatin1();

        const KDSoapValueList& values = requestMsg.childValues();

        // TODO use a cache like in QDBusConnectionPrivate::activateCall?
        const QMetaObject* mo = m_serverObject->metaObject();
        QList<QByteArray> qtTypes;
        //QList<int> metaTypes; // TODO needed?

        QVarLengthArray<void *, 10> params;
        params.reserve(values.count());
        // first one is the return type -- add it below
        params.append(0);

        QVector<QVariant> argValues;
        argValues.resize(values.count());
        qDebug() << "method=" << method << values.count() << "values";
        for (int i = 0; i < values.count(); ++i) {
            const KDSoapValue& soapValue = values.at(i);
            const QVariant& value = soapValue.value();
            // use type information if provided (use=encoded)
            QByteArray qtType;
            //int typeId = -1;
            if (soapValue.typeNs() == KDSoapNamespaceManager::xmlSchema1999() ||
                    soapValue.typeNs() == KDSoapNamespaceManager::xmlSchema2001()) {
                qtType = xmlTypeToVariant(soapValue.type());
                //typeId = kdSoapNameToTypeId(qtType.constData());
            } else {
                qtType = value.typeName();
                //typeId = value.userType();
            }
            qtTypes.append(qtType);
            // TODO error handling
            //Q_ASSERT(typeId != -1);
            Q_ASSERT(!qtType.isEmpty());
            //metaTypes.append(typeId);
            argValues[i] = value;
            params.append(const_cast<void *>(argValues.at(i).constData()));
            qDebug() << value;
        }
        int returnMetaType = -1;
        const int slotIdx = ::findSlot(mo, method, qtTypes, &returnMetaType);
        if (slotIdx < 0) {
            qDebug() << "Slot not found:" << mo->className() << method << qtTypes;
        }

        // output arguments
        QVariantList outputArgs;
        void *null = 0;
        if (returnMetaType != QMetaType::Void) {
            //qDebug() << "return type is" << returnMetaType;
            QVariant arg(returnMetaType, null);
            outputArgs.append(arg);
            params[0] = const_cast<void*>(outputArgs.at(0).constData());
        }
#if 0 // other output arguments, not sure we support this
        for ( ; i < metaTypes.count(); ++i) {
            QVariant arg(metaTypes[i], null);
            outputArgs.append( arg );
            params.append(const_cast<void*>(outputArgs.at( outputArgs.count() - 1 ).constData()));
        }
#endif
        const bool callOK = m_serverObject->qt_metacall(QMetaObject::InvokeMetaMethod, slotIdx, params.data()) < 0;

        if (callOK) {

            if (serverObjectInterface->hasFault()) {
                // TODO call getFault
                qDebug() << "Got fault!";
                // TODO set attributes in replyMsg
                replyMsg.setFault(true);
            } else {

                qDebug() << "Got return value" << outputArgs[0];
                // TODO FooResponse element
                replyMsg.setValue(outputArgs[0]);
            }
        } else {
            // TODO error handling for callOK, log error

            // TODO show full signature, it could be a problem with the argument types
            qDebug() << "Method not found:" << method << "in" << m_serverObject;
        }
    }

    // send replyMsg on socket
#if 0 // TODO
    const QByteArray response = makeHttpResponse(replyMsg.toXml());
    const bool doDebug = true; // TODO
    if (doDebug) {
        qDebug() << "HttpServerThread: writing" << response;
    }
#endif
    write(response);
    // flush() ?
}

#include "moc_KDSoapServerSocket_p.cpp"
