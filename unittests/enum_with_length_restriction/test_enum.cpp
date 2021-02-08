/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2014-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
**
****************************************************************************/

#include "httpserver_p.h"
#include <QTest>
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

    KDSoapValue soapValue = ar.serialize(QString());
    ar.deserialize(soapValue);

    QCOMPARE(ar.type(), TNS__AudienceRating::_6Bis);
}

QTEST_MAIN(TestEnum)

#include "test_enum.moc"
