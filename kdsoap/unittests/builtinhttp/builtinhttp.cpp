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
        XSD1__EmployeeType employeeType;
        employeeType.setType(XSD1__EmployeeTypeEnum::Developer);
        employeeType.setOtherRoles(QList<XSD1__EmployeeTypeEnum>() << XSD1__EmployeeTypeEnum::TeamLeader);
        service.addEmployee(employeeType, QString::fromLatin1("David"));
    }
};

QTEST_MAIN(TestObject)

#include "builtinhttp.moc"
