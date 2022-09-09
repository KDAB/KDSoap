/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "KDSoapServerCustomVerbRequestInterface.h"

KDSoapServerCustomVerbRequestInterface::KDSoapServerCustomVerbRequestInterface()
    : d(nullptr)
{
}

KDSoapServerCustomVerbRequestInterface::~KDSoapServerCustomVerbRequestInterface()
{
}

bool KDSoapServerCustomVerbRequestInterface::processCustomVerbRequest(const QByteArray &requestType, const QByteArray &requestData,
                                                                      const QMap<QByteArray, QByteArray> &httpHeaders, QByteArray &customAnswer)
{
    Q_UNUSED(requestType);
    Q_UNUSED(requestData);
    Q_UNUSED(httpHeaders);
    Q_UNUSED(customAnswer);

    return false;
}
