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
#ifndef KDSOAPCLIENTINTERFACE_H
#define KDSOAPCLIENTINTERFACE_H

#include <QtCore/QtGlobal>
#include <QtCore/QString>
#include "KDSoapMessage.h"
#include "KDSoapPendingCall.h"

class KDSoapAuthentication;
class KDSoapSslHandler;
class KDSoapClientInterfacePrivate;
QT_BEGIN_NAMESPACE
class QSslError;
class QSslConfiguration;
class QNetworkCookieJar;
class QNetworkProxy;
QT_END_NAMESPACE

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
    explicit KDSoapClientInterface(const QString &endPoint, const QString &messageNamespace);
    /**
     * Destroys the object interface and frees up any resource used.
     * \warning Any running asynchronous calls will be canceled.
     */
    ~KDSoapClientInterface();

    /**
     * Calls the method \p method on this interface and passes the arguments specified in \p message
     * to the method.
     * \param method method name, without arguments. For instance \c "addContact". Only used in RPC style.
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
    KDSoapPendingCall asyncCall(const QString &method, const KDSoapMessage &message,
                                const QString &soapAction = QString(),
                                const KDSoapHeaders &headers = KDSoapHeaders());

    /**
     * Calls the method \p method on this interface and passes the parameters specified in \p message
     * to the method.
     * \param method method name, without arguments. For instance \c "addContact". Only used in RPC style.
     * \param message arguments for the method call
     * \param soapAction optional \c "SoapAction" header, see the specification of the SOAP service.
     * \param headers optional arguments which will be passed as \c <soap:Header>.
     *
     * \warning This is a blocking call. It is NOT recommended to use this in the main thread of
     * graphical applications, since it will block the event loop for the duration of the call.
     * Use this only in threads, or in non-GUI programs.
     */
    KDSoapMessage call(const QString &method, const KDSoapMessage &message,
                       const QString &soapAction = QString(),
                       const KDSoapHeaders &headers = KDSoapHeaders());

    /**
     * Calls the method \p method on this interface and passes the parameters specified in \p message
     * to the method.
     * \param method method name, without arguments. For instance \c "addContact". Only used in RPC style.
     * \param message arguments for the method call
     * \param soapAction optional \c "SoapAction" header, see the specification of the SOAP service.
     * \param headers optional arguments which will be passed as \c <soap:Header>.
     *
     * This is an asynchronous call, where the caller does not want to wait for a response.
     * The method returns immediately, the call is performed later. No error handling is possible.
     */
    void callNoReply(const QString &method, const KDSoapMessage &message,
                     const QString &soapAction = QString(),
                     const KDSoapHeaders &headers = KDSoapHeaders());

    /**
     * Provide the necessary authentication for this service.
     * \param authentication the authentication data
     */
    void setAuthentication(const KDSoapAuthentication &authentication);

    /**
     * Sets a persistent header, which will be sent with any subsequent SOAP call.
     * \param name internal name, used to replace any existing header previously set with this name
     * \param header the actual message to be sent
     */
    void setHeader(const QString &name, const KDSoapMessage &header);

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
     * Returns the end point of the SOAP service.
     * \since 1.2
     */
    QString endPoint() const;

    /**
     * Sets the end point of the SOAP service.
     * \param endPoint the URL of the SOAP service, including http or https scheme, port number
     *                 if needed, and path. Example: http://server/path/soap.php
     * \since 1.2
     */
    void setEndPoint(const QString &endPoint);

    /**
     * Returns the cookie jar to use for the HTTP requests.
     * If no cookie jar was set by setCookieJar previously, a default
     * one will be returned, which belongs to the client interface (no need to delete it).
     * \since 1.2
     */
    QNetworkCookieJar *cookieJar() const;

    /**
     * Sets the cookie jar to use for the HTTP requests.
     * The ownership of the cookie jar is NOT transferred, so that it is possible
     * to share the same cookie jar between multiple client interfaces.
     * \since 1.2
     */
    void setCookieJar(QNetworkCookieJar *jar);

    /**
      * Returns the network proxy used for the HTTP requests.
      * \since 1.2
      * \sa QNetworkAccessManager::proxy()
      */
    QNetworkProxy proxy() const;

    /**
      * Sets the network proxy used for the HTTP requests.
      * \since 1.2
      * \sa QNetworkAccessManager::setProxy()
      */
    void setProxy(const QNetworkProxy &proxy);

    /**
      * Sets additional HTTP headers in the requests
      * \since 1.4
      * \sa QNetworkAccessManager::setRawHeader()
      */
    void setRawHTTPHeaders(const QMap<QByteArray, QByteArray> &headers);

    /**
     * WSDL style. See the "style" attribute for soap:binding, in the WSDL file.
     * See http://www.ibm.com/developerworks/webservices/library/ws-whichwsdl/ for a discussion
     * on the pros and cons of both styles.
     *
     * In RPC style, the method name passed to call() or asyncCall() is sent as an xml element
     * wrapping the message parameters.
     *
     * In Document style, the KDSoapMessage represents the entire "document" to be sent, so the
     * the method name passed to call() or asyncCall() is ignored, and the name of the KDSoapMessage
     * is used as the main xml element name. This difference is mostly useful in the case of
     * generated code, so that it can serialize existing complex types, and send them as messages.
     */
    enum Style {
        RPCStyle,       ///< the method name is sent as an xml element wrapping the message parameters
        DocumentStyle   ///< the message is sent as is, the method name is usually the name of the message
    };

    /**
     * Sets the WSDL style used by this service.
     * \since 1.1
     */
    void setStyle(Style style);

    /**
     * Returns the WSDL style used by this service.
     * \since 1.1
     */
    Style style() const;

    /**
     * Returns the headers returned by the last synchronous call().
     * For asyncCall(), use KDSoapPendingCall::returnHeaders().
     * \since 1.1
     */
    KDSoapHeaders lastResponseHeaders() const;

    /**
     * Asks Qt to ignore ssl errors in https requests. Use this for testing
     * only!
     */
    void ignoreSslErrors();

#ifndef QT_NO_OPENSSL
    /**
     * \brief ignoreSslErrors
     * If this function is called, the SSL errors given in \p errors will be ignored.
     * Note that you can set the expected certificate in the SSL error.
     * See QNetworkReply::ignoreSslErrors() for more information.
     *
     * \param errors list of errors to ignore
     * \since 1.4
     */
    void ignoreSslErrors(const QList<QSslError> &errors);
#endif

    /**
     * Returns the ssl handler object, which can be used for notification
     * and handling of SSL errors.
     *
     * Note that the notifications from synchronous calls will come in delayed, after the
     * synchronous call failed. For this reason, it is not possible to ignore specific
     * ssl errors during runtime when using synchronous calls. Use asynchronous calls instead.
     *
     * \since 1.3
     */
    KDSoapSslHandler *sslHandler() const;

#ifndef QT_NO_OPENSSL
    /**
     * Returns the ssl configuration used for outgoing connections
     * \since 1.3
     */
    QSslConfiguration sslConfiguration() const;

    /**
     * Sets the ssl configuration used for outgoing connections
     * \since 1.3
     */
    void setSslConfiguration(const QSslConfiguration &config);
#endif

private:
    friend class KDSoapThreadTask;

    KDSoapClientInterfacePrivate *const d;
};

#endif // KDSOAPCLIENTINTERFACE_H
