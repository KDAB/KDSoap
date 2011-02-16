#ifndef KDSOAPSERVEROBJECTINTERFACE_H
#define KDSOAPSERVEROBJECTINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QObject>

/**
 * Base class for server objects, i.e. objects implementing the methods
 * that can be called by SOAP clients.
 *
 * Your server object must derive from both QObject (directly or indirectly)
 * and from KDSoapServerObjectInterface, and each method it publishes to SOAP should
 * be declared as a slot.
 * Example:
 * <code>
  class EmployeeServerObject : public QObject, public KDSoapServerObjectInterface
  {
    Q_OBJECT
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
    //
    KDSoapServerObjectInterface();

};

Q_DECLARE_INTERFACE(KDSoapServerObjectInterface,
                    "com.kdab.KDSoap.ServerObjectInterface/1.0")

#endif // KDSOAPSERVEROBJECTINTERFACE_H
