/****************************************************************************
** Copyright (C) 2010-2011 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL.txt included.
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
#include <KDSoapMessage.h>
#include "KDSoapDelayedResponseHandle.h"
#include <QtCore/QObject>
class KDSoapServerSocket;

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
 * Example for manual handling:
  <code>
  class EmployeeServerObject : public QObject, public KDSoapServerObjectInterface
  {
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)

  public: // methods published to SOAP
    QString getEmployeeCountry(const QString& employeeName);
    [...]
  };
  </code>
  And in the .cpp file:
  <code>
    #define KDWSDLServerClass EmployeeServerObject
    #include "swsdl_employee.cpp"
  </code>
 *
 * swsdl_employee.cpp will contain the code which calls getEmployeeCountry
 * when parsing a KDSoapMessage that is a "getEmployeeCountry" request.
 *
 * Multi-threading note: KDSoapServer will create one instance of a "server object"
 * per thread. So the code in this class does not need to be protected for thread
 * safety. Make sure to protect any shared resources though.
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
     * When using kdwsdl2cpp to generate the server-side code, this is the method
     * that will be generated, so that you don't have to implement it, only the pure
     * virtual methods called by processRequest.
     */
    virtual void processRequest(const KDSoapMessage& request, KDSoapMessage& response, const QByteArray& soapAction);

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
    void setResponseHeaders(const KDSoapHeaders& headers);

    /**
     * Instructs KD SOAP to return a fault message instead of the return value of the slot.
     *
     * \param faultCode A code for identifying the fault. Example: "Server.EntryNotFound", or
     *                  "Client.Authentication". Must not be empty.
     * \param faultString A human readable explanation of the fault
     * \param faultActor Information about who caused the fault to happen
     * \param Holds application specific error information related to the Body element
     *
     * See http://www.w3.org/TR/2000/NOTE-SOAP-20000508/#_Toc478383507 for more details.
     */
    void setFault(const QString& faultCode, const QString& faultString, const QString& faultActor, const QString& detail);

    /**
     * Returns true if setFault was called in the current method invocation.
     */
    bool hasFault() const;

    /**
     * When a server object wants to implement a SOAP method
     * call using an asynchronous operation (I/O or network for instance),
     * it should call prepareDelayedResponse() from within the call handler, store
     * the handle, return a dummy value (this allows to go back to the event loop),
     * and use the handle later on (typically from a slot) in order to send the delayed response.
     */
    KDSoapDelayedResponseHandle prepareDelayedResponse(); // only valid during processRequest()
    /**
     * Returns true if prepareDelayedResponse was called, during this soap call.
     * Mostly useful internally in KDSoap.
     */
    bool isDelayedResponse() const; // only valid during processRequest()

    /**
     * Send a delayed response.
     * \param responseHandle the identifier of the call we are responding to
     * \param response the response message for that call
     */
    void sendDelayedResponse(const KDSoapDelayedResponseHandle& responseHandle, const KDSoapMessage& response);

private:
    friend class KDSoapServerSocket;
    void setServerSocket(KDSoapServerSocket* serverSocket); // only valid during processRequest()
    void setRequestHeaders(const KDSoapHeaders& headers, const QByteArray& soapAction);
    KDSoapHeaders responseHeaders() const;
    void storeFaultAttributes(KDSoapMessage& message) const;
    class Private;
    Private* const d;
};

Q_DECLARE_INTERFACE(KDSoapServerObjectInterface,
                    "com.kdab.KDSoap.ServerObjectInterface/1.0")

#endif // KDSOAPSERVEROBJECTINTERFACE_H
