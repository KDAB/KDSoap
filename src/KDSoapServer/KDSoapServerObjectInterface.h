/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/
#ifndef KDSOAPSERVEROBJECTINTERFACE_H
#define KDSOAPSERVEROBJECTINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <KDSoapClient/KDSoapMessage.h>
#include "KDSoapDelayedResponseHandle.h"

#include <QtCore/QObject>
#include <QIODevice>

class KDSoapServerSocket;
class QAbstractSocket;

/**
 * Base class for server objects, i.e. objects implementing the methods
 * that can be called by SOAP clients.
 *
 * Your server object must derive from both QObject (directly or indirectly)
 * and from KDSoapServerObjectInterface.
 * The virtual method processRequest is called whenever a SOAP request is being made.
 * To handle the call, either reimplement processRequest and do the dispatching
 * manually (not recommended), or use kdwsdl2cpp to generate the base class for your server object.
 *
 * Example:
  <code>
  class EmployeeServerObject : public MyServerBase // base class generated from the wsdl file
  {
    Q_OBJECT

  public: // methods published to SOAP
    QString getEmployeeCountry(const QString& employeeName);
    [...]
  };
  </code>
 *
 * The generated base class provides the pure virtual methods for each method
 * defined in the WSDL file, such as getEmployeeCountry() in this example,
 * as well as a generated processRequest() method which calls getEmployeeCountry
 * when parsing a KDSoapMessage that is a "getEmployeeCountry" request.
 *
 * Multi-threading note: KDSoapServer will create one instance of a "server object"
 * per thread. So the code in this class does not need to be protected for thread-safety.
 * Make sure to protect any shared resources though.
 */
class KDSOAPSERVER_EXPORT KDSoapServerObjectInterface
{
public:
    /**
     * Constructor
     */
    KDSoapServerObjectInterface();
    /**
     * Destructor
     */
    virtual ~KDSoapServerObjectInterface();

    /**
     * Handle \p request and return \p response.
     * The default implementation in this base class is to simply return
     * a fault "method not found". Subclasses must implement the dispatching
     * to actual implementation methods.
     *
     * This method is called when calls are made to the default path for the server,
     * as configured in KDSoapServer. For other paths, see processRequestWithPath().
     *
     * When using kdwsdl2cpp to generate the server-side code, this is the method
     * that will be generated, so that you don't have to implement it, only the pure
     * virtual methods called by processRequest.
     */
    virtual void processRequest(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction);

    /**
     * Handle an HTTP GET request, typically by returning a file.
     *
     * If a NULL device is returned by this method, a 404 error will be returned to the client.
     *
     * \param path the HTTP path sent by the client (this could also include a fragment and/or a query, see QUrl)
     * \param contentType output variable, your method should set it to the content type header to send back with the file.
     *       For instance "text/plain" for a plain text file.
     * \return an iodevice for reading from. For instance a new QFile.
     * KDSoap will delete the iodevice after reading all its contents.
     * \since 1.3
     */
    virtual QIODevice *processFileRequest(const QString &path, QByteArray &contentType);

    /**
     * Handle a SOAP request that was sent to a different path than the one configured in KDSoapServer.
     *
     * \param request the incoming SOAP request
     * \param response the SOAP response to be sent back
     * \param soapAction the SOAP action string sent by the client
     * \param path the HTTP path sent by the client (this could also include a fragment and/or a query, see QUrl)
     *
     * Note that a GET request with Accept="application/soap+xml" will trigger a call to this method with an empty
     * incoming soap request, in order to make it possible to return SOAP responses. This is as documented in
     * http://www.ibm.com/developerworks/xml/library/x-tipgetr/.
     *
     * The default implementation in this base class is to simply return
     * a fault "method not found". Subclasses must implement the dispatching
     * to actual implementation methods. Usually this will be done by calling processRequest
     * on other KDSoapServerObjectInterface instances, for instance generated by kdwsdl2cpp.
     * \since 1.3
     */
    virtual void processRequestWithPath(const KDSoapMessage &request, KDSoapMessage &response, const QByteArray &soapAction, const QString &path);

    /**
     * Call this after processRequestWithPath has finished handling a request,
     * in order to copy response headers, faults, etc. from the secondary object interface
     * into this one.
     * \since 1.4
     */
    void doneProcessingRequestWithPath(const KDSoapServerObjectInterface &otherInterface);

    /**
     * Returns the SOAP headers that were provided together with the SOAP request.
     * This can be used to retrieve authentication headers, or any kind of session
     * (per-client) data.
     */
    KDSoapHeaders requestHeaders() const;

    /**
     * Returns the "Soap Action" header sent by the client.
     */
    QByteArray soapAction() const;

    /**
     * Sets the soap headers to be sent in the response
     */
    void setResponseHeaders(const KDSoapHeaders &headers);

    /**
     * Sets the message namespace to be used in the response.
     * If the requests comes with a message namespace("qualified"), then this can be determined from that namespace.
     * But if the request is not qualified, this is very much necessary (at least when the response has headers,
     * which are always qualified).
     * \since 1.2
     */
    void setResponseNamespace(const QString &ns);

    /**
     * Instructs KD SOAP to return a fault message instead of the return value of the slot.
     *
     * \param faultCode A code for identifying the fault. Example: "Server.EntryNotFound", or
     *                  "Client.Authentication". Must not be empty.
     * \param faultString A human-readable explanation of the fault
     * \param faultActor Information about who caused the fault to happen
     * \param detail Holds application-specific error information related to the Body element
     *
     * See http://www.w3.org/TR/2000/NOTE-SOAP-20000508/#_Toc478383507 for more details.
     */
    void setFault(const QString &faultCode, const QString &faultString, const QString &faultActor = QString(), const QString &detail = QString());

    /**
     * Instructs KD SOAP to return a fault message instead of the return value of the slot.
     *
     * \param faultCode A code for identifying the fault. Example: "Server.EntryNotFound", or
     *                  "Client.Authentication". Must not be empty.
     * \param faultString A human-readable explanation of the fault
     * \param faultActor Information about who caused the fault to happen
     * \param detail Holds application-specific error information related to the Body element, it is given as a KDSoapValue and hence can be parsed
     *
     * See http://www.w3.org/TR/2000/NOTE-SOAP-20000508/#_Toc478383507 for more details.
     */
    void setFault(const QString &faultCode, const QString &faultString, const QString &faultActor, const KDSoapValue &detail);

    /**
     * Returns true if setFault was called in the current method invocation.
     */
    bool hasFault() const;

    /**
     * Returns a pointer to the server socket. Only valid during processRequest().
     * This can be used to retrieve information from the server socket, such as peerAddress etc.
     * \since 1.3
     */
    QAbstractSocket *serverSocket() const;

    /**
     * When a server object wants to implement a SOAP method
     * call using an asynchronous operation (I/O or network for instance),
     * it should call prepareDelayedResponse() from within the call handler, store
     * the handle, return a dummy value (this allows to go back to the event loop),
     * and use the handle later on (typically from a slot) in order to send the delayed response.
     * \since 1.2
     */
    KDSoapDelayedResponseHandle prepareDelayedResponse(); // only valid during processRequest()
    /**
     * Returns true if prepareDelayedResponse was called, during this soap call.
     * Mostly useful internally in KDSoap.
     * \since 1.2
     */
    bool isDelayedResponse() const; // only valid during processRequest()

    /**
     * Send a delayed response.
     * \param responseHandle the identifier of the call we are responding to
     * \param response the response message for that call
     * \since 1.2
     */
    void sendDelayedResponse(const KDSoapDelayedResponseHandle &responseHandle, const KDSoapMessage &response);

    /**
     * Low-level method, not needed for normal operations.
     * Call this method to write an HTTP reply back, e.g. in case of an error.
     * Example: writeHTTP("HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n");
     * \since 1.5
     */
    void writeHTTP(const QByteArray &httpReply);

    /**
     * Low-level method, not needed for normal operations.
     * Call this method to write an XML reply back. KDSoapServer will take care
     * of the necessary HTTP headers.
     * @param reply the SOAP reply in XML
     * @param isFault true if the reply is a fault (which means sending back the HTTP error code 500, as per the specification)
     * \since 1.5
     */
    void writeXML(const QByteArray &reply, bool isFault = false);

private:
    friend class KDSoapServerSocket;
    void setServerSocket(KDSoapServerSocket *serverSocket); // only valid during processRequest()
    void setRequestHeaders(const KDSoapHeaders &headers, const QByteArray &soapAction);
    KDSoapHeaders responseHeaders() const;
    QString responseNamespace() const;
    void storeFaultAttributes(KDSoapMessage &message) const;
    class Private;
    Private *const d;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(KDSoapServerObjectInterface,
                    "com.kdab.KDSoap.ServerObjectInterface/1.0")
QT_END_NAMESPACE

#endif // KDSOAPSERVEROBJECTINTERFACE_H
