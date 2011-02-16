#include "KDSoapServerSocket_p.h"
#include "KDSoapSocketList_p.h"
#include "KDSoapServerObjectInterface.h"
#include <KDSoapMessage.h>
#include <QBuffer>

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
        //KDSoapServerObjectInterface* serverObjectInterface = qobject_cast<KDSoapServerObjectInterface *>(m_serverObject);


        const QByteArray method = requestMsg.name().toLatin1();
        // TODO error handling, e.g. empty method, no such slot
        qDebug() << "method=" << method;

        // TODO find the slot, see QDBusConnectionPrivate::activateCall
        // It also uses a cache, interesting idea.
        // And then we shouldn't use invokeMethod, it looks up the method again
        // ==> use qt_metacall()

        QVector<QVariant> argValues;
        QString retval;
        QGenericReturnArgument ret("QString", &retval); // TODO determine from method signature
        QGenericArgument args[10];
        const KDSoapValueList& values = requestMsg.childValues();
        argValues.resize(values.count());
        for (int i = 0; i < values.count(); ++i) {
            const KDSoapValue& soapValue = values.at(i);
            const QVariant& value = soapValue.value();
            //const int variantType = value.userType();
            // TODO type conversion if necessary
            argValues[i] = value;
            args[i] = QGenericArgument(value.typeName() /*must match method arg type*/, argValues.at(i).constData());
        }

        bool callOK = QMetaObject::invokeMethod(m_serverObject, method.constData(), Qt::DirectConnection, ret, args[0], args[1], args[2], args[3], args[4], args[5], args[6], args[7], args[8], args[9]);
        if (callOK) {
            qDebug() << "Got return value" << retval;
            // TODO FooResponse element
            replyMsg.setValue(retval);
        } else {
            // TODO error handling for callOK, log error
            qDebug() << "Method not found:" << method << "in" << m_serverObject;
        }

#if 0
        // From qdbusintegrator.cpp:

        QVarLengthArray<void *, 10> params;
         params.reserve(metaTypes.count());

         QVariantList auxParameters;
         // let's create the parameter list

         // first one is the return type -- add it below
         params.append(0);

         // add the input parameters
         int i;
         int pCount = qMin(msg.arguments().count(), metaTypes.count() - 1);
         for (i = 1; i <= pCount; ++i) {
             int id = metaTypes[i];
             const QVariant &arg = msg.arguments().at(i - 1);
             if (arg.userType() == id)
                 // no conversion needed
                 params.append(const_cast<void *>(arg.constData()));
             else if (arg.userType() == qMetaTypeId<QDBusArgument>()) {
                 // convert to what the function expects
                 void *null = 0;
                 auxParameters.append(QVariant(id, null));

                 const QDBusArgument &in =
                     *reinterpret_cast<const QDBusArgument *>(arg.constData());
                 QVariant &out = auxParameters[auxParameters.count() - 1];

                 if (!QDBusMetaType::demarshall(in, out.userType(), out.data()))
                     qFatal("Internal error: demarshalling function for type '%s' (%d) failed!",
                            out.typeName(), out.userType());

                 params.append(const_cast<void *>(out.constData()));
             } else {
                 qFatal("Internal error: got invalid meta type %d (%s) "
                        "when trying to convert to meta type %d (%s)",
                        arg.userType(), QMetaType::typeName(arg.userType()),
                        id, QMetaType::typeName(id));
             }
         }

        // output arguments
        QVariantList outputArgs;
        void *null = 0;
        if (metaTypes[0] != QMetaType::Void) {
            QVariant arg(metaTypes[0], null);
            outputArgs.append( arg );
            params[0] = const_cast<void*>(outputArgs.at( outputArgs.count() - 1 ).constData());
        }
        for ( ; i < metaTypes.count(); ++i) {
            QVariant arg(metaTypes[i], null);
            outputArgs.append( arg );
            params.append(const_cast<void*>(outputArgs.at( outputArgs.count() - 1 ).constData()));
        }
        bool fail = m_serverObject->qt_metacall(QMetaObject::InvokeMetaMethod, slotIdx, params.data()) >= 0;
#endif
    }

    // send replyMsg on socket
#if 0 // TODO
    // design: use a QIODevice?
    const QByteArray response = makeHttpResponse(replyMsg.toXml());
    const bool doDebug = true; // TODO
    if (doDebug) {
        qDebug() << "HttpServerThread: writing" << response;
    }
    write(response);
#endif
    // flush() ?
}

#include "moc_KDSoapServerSocket_p.cpp"
