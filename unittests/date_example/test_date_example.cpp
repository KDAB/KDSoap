/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapClientInterface.h"
#include "wsdl_date_example.h"
#include <QDebug>
#include <QEventLoop>
#include <QTest>

class DateExampleWSDL : public QObject
{
    Q_OBJECT
public:
private slots:
    void slotCheckCompilation()
    {
        DateExample service;
        StringToDateJob job(&service);
        QDate result = job.dateObject();
        QCOMPARE(result, QDate());
    }
};

QTEST_MAIN(DateExampleWSDL)

#include "test_date_example.moc"
