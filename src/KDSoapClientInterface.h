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
 */ // TODO code example
class Q_DECL_EXPORT KDSoapClientInterface
{
public:
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
     * Calls the method @p method on this interface and passes the parameters specified in @p message
     * to the method.
     * @param soapAction optional "SoapAction" header, see the specification of the SOAP service.
     *
     * This is an asynchronous call, so this function returns immediately.
     * The returned KDSoapPendingCall object can be used to find out information about the reply.
     * You should create a KDSoapPendingCallWatcher to connect to the finished() signal.
     *
     * Note that the returned KDSoapPendingCall object (or a copy of it) must stay alive
     * for the whole duration of the call. If you do not want to wait for a response,
     * use callNoReply instead.
     */ // TODO code example
    KDSoapPendingCall asyncCall(const QString& method, const KDSoapMessage &message,
                                const QString& soapAction = QString(),
                                const KDSoapHeaders& headers = KDSoapHeaders());

    /**
     * Calls the method @p method on this interface and passes the parameters specified in @p message
     * to the method.
     * @param soapAction optional "SoapAction" header, see the specification of the SOAP service.
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
     * @param soapAction optional "SoapAction" header, see the specification of the SOAP service.
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

private:
    friend class KDSoapThreadTask;

    class Private;
    Private * const d;
};

#endif // KDSOAPCLIENTINTERFACE_H
