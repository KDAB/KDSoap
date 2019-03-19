#include "wsdl_test_qsharedpointer_include_wsdl.h"

#include <QtTest>

class RightInclude:
  public QObject
{
Q_OBJECT

private Q_SLOTS:

  void testCompiled()
  {
    Hello_Service service;
  }
};

QTEST_MAIN(RightInclude)

#include "test_qsharedpointer_include.moc"
