/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2010-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: LicenseRef-KDAB-KDSoap-AGPL3-Modified OR LicenseRef-KDAB-KDSoap OR LicenseRef-KDAB-KDSoap-US
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
** Contact info@kdab.com if any conditions of this licensing are not clear to you.
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
