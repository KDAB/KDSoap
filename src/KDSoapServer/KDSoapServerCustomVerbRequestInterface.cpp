/****************************************************************************
** Copyright (C) 2010-2016 Klaralvdalens Datakonsult AB, a KDAB Group company, info@kdab.com.
** All rights reserved.
**
** This file is part of the KD Soap library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
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

#include "KDSoapServerCustomVerbRequestInterface.h"

KDSoapServerCustomVerbRequestInterface::KDSoapServerCustomVerbRequestInterface()
    : d(0)
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
