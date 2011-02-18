#ifndef KDSOAPSERVEROBJECTINTERFACE_H
#define KDSOAPSERVEROBJECTINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <KDSoapMessage.h>
#include <QtCore/QObject>

/**
 * Base class for server objects, i.e. objects implementing the methods
 * that can be called by SOAP clients.
 *
 * Your server object must derive from both QObject (directly or indirectly)
 * and from KDSoapServerObjectInterface, and each method it publishes to SOAP
 * should be declared as a slot.
 * Example:
 * <code>
  class EmployeeServerObject : public QObject, public KDSoapServerObjectInterface
  {
    Q_OBJECT
    Q_INTERFACES(KDSoapServerObjectInterface)

  public slots:
    QString getEmployeeCountry(const QString& employeeName) {
        return m_employeeCountryHash.value(employeeName);
    }
  };
 * </code>
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
     * Returns the SOAP headers that were provided together with the SOAP request.
     * This can be used to retrieve authentication headers, or any kind of session
     * (per-client) data.
     */
    KDSoapHeaders headers() const;

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

private:
    friend class KDSoapServerSocket;
    void setHeaders(const KDSoapHeaders& headers);
    void resetFault();
    bool hasFault() const;
    void storeFaultAttributes(KDSoapMessage& message) const;
    class Private;
    Private* const d;
};

Q_DECLARE_INTERFACE(KDSoapServerObjectInterface,
                    "com.kdab.KDSoap.ServerObjectInterface/1.0")

#endif // KDSOAPSERVEROBJECTINTERFACE_H
