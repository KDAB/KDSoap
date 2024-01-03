/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2014 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "httpserver_p.h"
#include "wsdl_test_enum.h"
#include <QTest>

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

    KDSoapValue soapValue = ar.serialize(QString());
    ar.deserialize(soapValue);

    QCOMPARE(ar.type(), TNS__AudienceRating::_6Bis);
}

QTEST_MAIN(TestEnum)

#include "test_enum.moc"
