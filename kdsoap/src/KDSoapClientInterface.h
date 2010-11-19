/****************************************************************************
** Copyright (C) 2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef KDSOAPCLIENTINTERFACE_H
#define KDSOAPCLIENTINTERFACE_H

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include "KDSoapMessage.h"
#include "KDSoapPendingCall.h"

class KDSoapAuthentication;

/**
 * KDSoapClientInterface is a generic accessor class that is used to place
 * calls to remote SOAP objects.
 * This class is useful for dynamic access to remote objects: that is, when
 * you do not have a generated code that represents the remote interface.
 * \code
 *  const int year = 2009;
 *
 *  const QString endPoint = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/USHolidayDates.asmx");
 *  const QString messageNamespace = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/");
 *  KDSoapClientInterface client(endPoint, messageNamespace);
 *
 *  KDSoapMessage message;
 *  message.addArgument(QLatin1String("year"), year);
 *
 *  qDebug("Looking up the date of Valentine's Day in %i...", year);
 *
 *  KDSoapMessage response = client.call(QLatin1String("GetValentinesDay"), message);
 *
 *  qDebug("%s", qPrintable(response.arguments()[0].value().toString()));
 * \endcode
 */
class KDSOAP_EXPORT KDSoapClientInterface
{
public:
    /**
     * Version of the SOAP protocol to use when sending requests.
     * \see setSoapVersion()
     */
    enum SoapVersion {
      /** Use format version 1.1 of the SOAP specification */
      SOAP1_1 = 1,
      /** Use format version 1.2 of the SOAP specification */
      SOAP1_2 = 2
    };

    /**
     * Creates a KDSoapClientInterface object associated with the end point \p endPoint.
     * \note No connection is done yet at this point, the parameters are simply stored for later use.
     * \param endPoint the URL of the SOAP service, including http or https scheme, port number
     *                 if needed, and path. Example: http://server/path/soap.php
     * \param messageNamespace the namespace URI used for the message and its arguments.
     *                 Example: http://server/path, but could be any URI, it doesn't have to exist
     *                 or even to be http, this is really just a namespace, which is part of the
     *                 specification of the SOAP service.
     */
    explicit KDSoapClientInterface(const QString& endPoint, const QString& messageNamespace);
    /**
     * Destroy the object interface and frees up any resource used.
     * \warning Any running asynchronous calls will be canceled.
     */
    ~KDSoapClientInterface();

    /**
     * Calls the method \p method on this interface and passes the arguments specified in \p message
     * to the method.
     * \param method method name, without arguments. For instance \c "addContact".
     * \param message arguments for the method call
     * \param soapAction optional \c "SoapAction" header, see the specification of the SOAP service.
     * \param headers optional arguments which will be passed as \c <soap:Header>.
     *
     * This is an asynchronous call, so this function returns immediately.
     * The returned KDSoapPendingCall object can be used to find out information about the reply.
     * You should create a KDSoapPendingCallWatcher to connect to the finished() signal.
     *
     * \warning The returned KDSoapPendingCall object (or a copy of it) must stay alive
     * for the whole duration of the call. If you do not want to wait for a response,
     * use callNoReply instead.
     *
     * \code
     *  const int year = 2009;
     *
     *  const QString endPoint = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/USHolidayDates.asmx");
     *  const QString messageNamespace = QLatin1String("http://www.27seconds.com/Holidays/US/Dates/");
     *  KDSoapClientInterface* client= new KDSoapClientInterface(endPoint, messageNamespace);
     *
     *  KDSoapMessage message;
     *  message.addArgument(QLatin1String("year"), year);
     *
     *  qDebug("Looking up the date of Valentine's Day in %i...", year);
     *
     *  KDSoapPendingCall pendingCall = client->asyncCall(QLatin1String("GetValentinesDay"), message);
     *
     *  // create a watcher object that will signal the call's completion
     *  KDSoapPendingCallWatcher* watcher = new KDSoapPendingCallWatcher(pendingCall, this);
     *  connect(watcher, SIGNAL(finished(KDSoapPendingCallWatcher*)),
     *          this, SLOT(pendingCallFinished(KDSoapPendingCallWatcher*)));
     *
     *  void MyClass::pendingCallFinished(KDSoapPendingCallWatcher* pendingCall)
     *  {
     *      KDSoapMessage response = pendingCall->returnMessage();
     *      qDebug("%s", qPrintable(response.arguments()[0].value().toString()));
     *  }
     * \endcode
     */
    KDSoapPendingCall asyncCall(const QString& method, const KDSoapMessage &message,
                                const QString& soapAction = QString(),
                                const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Calls the method \p method on this interface and passes the parameters specified in \p message
     * to the method.
     * \param method method name, without arguments. For instance \c "addContact".
     * \param message arguments for the method call
     * \param soapAction optional \c "SoapAction" header, see the specification of the SOAP service.
     * \param headers optional arguments which will be passed as \c <soap:Header>.
     *
     * \warning This is a blocking call. It is NOT recommended to use this in the main thread of
     * graphical applications, since it will block the event loop for the duration of the call.
     * Use this only in threads, or in non-GUI programs.
     */
    KDSoapMessage call(const QString& method, const KDSoapMessage &message,
                       const QString& soapAction = QString(),
                       const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Calls the method \p method on this interface and passes the parameters specified in \p message
     * to the method.
     * \param method method name, without arguments. For instance \c "addContact".
     * \param message arguments for the method call
     * \param soapAction optional \c "SoapAction" header, see the specification of the SOAP service.
     * \param headers optional arguments which will be passed as \c <soap:Header>.
     *
     * This is an asynchronous call, where the caller does not want to wait for a response.
     * The method returns immediately, the call is performed later. No error handling is possible.
     */
    void callNoReply(const QString& method, const KDSoapMessage &message,
                     const QString& soapAction = QString(),
                     const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Provide the necessary authentication for this service.
     * \param authentication the authentication data
     */
    void setAuthentication(const KDSoapAuthentication& authentication);

    /**
     * Sets a persistent header, which will be sent with any subsequent SOAP call.
     * \param name internal name, used to replace any existing header previously set with this name
     * \param header the actual message to be sent
     */
    void setHeader(const QString& name, const KDSoapMessage& header);

    /**
     * Sets the SOAP version to be used for any subsequent SOAP call.
     * \param version #SOAP1_1 or #SOAP1_2
     * The default version is SOAP 1.1.
     */
    void setSoapVersion(SoapVersion version);

    /**
     * Returns the version of SOAP being used in this instance.
     */
    SoapVersion soapVersion();

    /**
     * Asks Qt to ignore ssl errors in https requests. Use this for testing
     * only!
     */
    void ignoreSslErrors();

private:
    friend class KDSoapThreadTask;

    class Private;
    Private * const d;
};

#endif // KDSOAPCLIENTINTERFACE_H
