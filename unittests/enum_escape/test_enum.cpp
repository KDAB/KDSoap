/****************************************************************************
** Copyright (C) 2014-2016 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** This file may be distributed and/or modified under the terms of the
** GNU Lesser General Public License version 2.1 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.LGPL.txt included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

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
