#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "KDSoapValue.h"
#include "KDSoapPendingCallWatcher.h"
#include "wsdl_mywsdl.h"
#include <QtTest/QtTest>
#include <QEventLoop>
#include <QDebug>

// TODO: builtin http server

class TestObject : public QObject
{
    Q_OBJECT
public:

    void testMyWsdl()
    {
        MyWsdl service;
        KDAB__EmployeeType employeeType;
        employeeType.setType(KDAB__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<KDAB__EmployeeTypeEnum>() << KDAB__EmployeeTypeEnum::TeamLeader);
        service.addEmployee(employeeType, KDAB__EmployeeName(QString::fromLatin1("David")), QString::fromLatin1("France"));
    }
};

QTEST_MAIN(TestObject)

#include "builtinhttp.moc"
