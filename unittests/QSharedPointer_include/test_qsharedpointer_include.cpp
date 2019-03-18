#include "wsdl_test_qsharedpointer_include_wsdl.h"

#include <QtTest>

class rightInclude:
  public QObject
{
Q_OBJECT

private Q_SLOTS:

  void testCompiled()
  {
    QVERIFY(true);
  }
};

QTEST_MAIN(rightInclude)

#include "test_qsharedpointer_include.moc"
