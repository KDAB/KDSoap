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
 * @code
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
 * @endcode
 */
class KDSOAP_EXPORT KDSoapClientInterface
{
public:
    enum SoapVersion{ SOAP1_1, SOAP1_2 };
    
    /**
     * Creates a KDSoapClientInterface object associated with the end point @p endPoint.
     * No connection is done yet at this point, the parameters are simply stored for later use.
     * @param endPoint the URL of the SOAP service, including http or https scheme, port number
     *                 if needed, and path. Example: http://server/path
     * @param messageNamespace the namespace URI used for the message and its arguments.
     *                 Example: http://server/path, but could be any URI, it doesn't have to exist
     *                 or even to be http, this is really just a namespace, which is part of the
     *                 specification of the SOAP service.
     */
    explicit KDSoapClientInterface(const QString& endPoint, const QString& messageNamespace);
    /**
     * Destroy the object interface and frees up any resource used.
     * Any running asynchronous calls will be canceled.
     */
    ~KDSoapClientInterface();

    /**
     * Calls the method @p method on this interface and passes the arguments specified in @p message
     * to the method.
     * @param method method name, without arguments. For instance "addContact".
     * @param message arguments for the method call
     * @param soapAction optional "SoapAction" header, see the specification of the SOAP service.
     * @param headers optional arguments which will be passed as <soap:Header>.
     *
     * This is an asynchronous call, so this function returns immediately.
     * The returned KDSoapPendingCall object can be used to find out information about the reply.
     * You should create a KDSoapPendingCallWatcher to connect to the finished() signal.
     *
     * Note that the returned KDSoapPendingCall object (or a copy of it) must stay alive
     * for the whole duration of the call. If you do not want to wait for a response,
     * use callNoReply instead.
     *
     * @code
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
     * KDSoapPendingCall pendingCall = client->asyncCall(QLatin1String("GetValentinesDay"), message);
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
     * @endcode
     */
    KDSoapPendingCall asyncCall(const QString& method, const KDSoapMessage &message,
                                const QString& soapAction = QString(),
                                const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Calls the method @p method on this interface and passes the parameters specified in @p message
     * to the method.
     * @param method method name, without arguments. For instance "addContact".
     * @param message arguments for the method call
     * @param soapAction optional "SoapAction" header, see the specification of the SOAP service.
     * @param headers optional arguments which will be passed as <soap:Header>.
     *
     * This is a blocking call. It is NOT recommended to use this in the main thread of
     * graphical applications, since it will block the event loop for the duration of the call.
     * Use this only in threads, or in non-GUI programs.
     */
    KDSoapMessage call(const QString& method, const KDSoapMessage &message,
                       const QString& soapAction = QString(),
                       const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Calls the method @p method on this interface and passes the parameters specified in @p message
     * to the method.
     * @param method method name, without arguments. For instance "addContact".
     * @param message arguments for the method call
     * @param soapAction optional "SoapAction" header, see the specification of the SOAP service.
     * @param headers optional arguments which will be passed as <soap:Header>.
     *
     * This is an asynchronous call, where the caller does not want to wait for a response.
     * The method returns immediately, the call is performed later. No error handling is possible.
     */
    void callNoReply(const QString& method, const KDSoapMessage &message,
                     const QString& soapAction = QString(),
                     const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Provide the necessary authentication for this service.
     */
    void setAuthentication(const KDSoapAuthentication& authentication);

    /**
     * Sets a persistent header, which will be sent with any subsequent soap call.
     * @param name internal name, used to replace any existing header previously set with this name
     * @param header the actual message to be sent
     */
    void setHeader(const QString& name, const KDSoapMessage& header);

    /**
     * Sets the version of the Soap to be sent with any subsequent soap call.
     * @param version KDSoapClientInterface::SoapVersion::SOAP1_1 or SOAP1_2
     */
    void setSoapVersion(SoapVersion version);
    
    /**
     * gets the version of Soap being used in this instance.
     */
    SoapVersion soapVersion();
    
private:
    friend class KDSoapThreadTask;
    
    class Private;
    Private * const d;
};

#endif // KDSOAPCLIENTINTERFACE_H
