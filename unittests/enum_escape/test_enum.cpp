#include "httpserver_p.h"
#include <QtTest/QtTest>
#include "wsdl_test_enum.h"

class TestEnum : public QObject
{
    Q_OBJECT
public:
    explicit TestEnum();

private slots:
    void test();
};

using namespace KDSoapUnitTestHelpers;

TestEnum::TestEnum()
{
}

void TestEnum::test()
{
  // bunch of tests to prove numbers are well handled
  TNS__AudienceRating ar;
  ar.setType(TNS__AudienceRating::_12);
  ar.setType(TNS__AudienceRating::_6);
  ar.setType(TNS__AudienceRating::_18);
  ar.setType(TNS__AudienceRating::_16);
  ar.setType(TNS__AudienceRating::Http___f_q_d_n_ext_enum2);
  ar.setType(TNS__AudienceRating::_6Bis);

  QVariant var = ar.serialize();
  ar.deserialize(var);

  QCOMPARE(ar.type(), TNS__AudienceRating::_6Bis);
}

QTEST_MAIN(TestEnum)

#include "test_enum.moc"
