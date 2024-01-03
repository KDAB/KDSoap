/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPPENDINGCALLWATCHER_P_H
#define KDSOAPPENDINGCALLWATCHER_P_H

// currently unused
class KDSoapPendingCallWatcher::Private
{
public:
    Private(KDSoapPendingCallWatcher *qq)
        : q(qq)
    {
    }

    KDSoapPendingCallWatcher *q;
};

#endif // KDSOAPPENDINGCALLWATCHER_P_H
