/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2010-2023 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef KDSOAPREPLYSSLHANDLER_P_H
#define KDSOAPREPLYSSLHANDLER_P_H

#include <QObject>
#include <QSslError>
QT_BEGIN_NAMESPACE
class QNetworkReply;
QT_END_NAMESPACE

class KDSoapSslHandler;

#ifndef QT_NO_SSL

class KDSoapReplySslHandler : public QObject
{
    Q_OBJECT
public:
    explicit KDSoapReplySslHandler(QNetworkReply *reply, KDSoapSslHandler *handler);

private Q_SLOTS:
    void slotReplySslErrors(const QList<QSslError> &errors);

private:
    KDSoapSslHandler *m_handler;
};

#endif
#endif // KDSOAPREPLYSSLHANDLER_P_H
