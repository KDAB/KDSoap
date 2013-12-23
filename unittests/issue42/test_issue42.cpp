#include "httpserver_p.h"
#include <QtTest/QtTest>
#include <QDebug>
#include "wsdl_test_number.h"

class TestIssue42 : public QObject
{
    Q_OBJECT
public:
    explicit TestIssue42();

private slots:
    void test();
};

using namespace KDSoapUnitTestHelpers;

TestIssue42::TestIssue42()
{
}

void TestIssue42::test()
{
  // bunch of tests to prove numbers are well handled
  TNS__AudienceRating ar;
  ar.setType(TNS__AudienceRating::_12);
  ar.setType(TNS__AudienceRating::_6);
  ar.setType(TNS__AudienceRating::_18);
  ar.setType(TNS__AudienceRating::_16);

  QVariant var = ar.serialize();
  ar.deserialize(var);

  QVERIFY (ar.type() == TNS__AudienceRating::_16 );
}

QTEST_MAIN(TestIssue42)

#include "test_issue42.moc"
