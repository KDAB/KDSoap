/****************************************************************************
**
** This file is part of the KD Soap project..
**
** SPDX-FileCopyrightText: 2010-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/
#ifndef KDSOAPSERVERAUTHINTERFACE_H
#define KDSOAPSERVERAUTHINTERFACE_H

#include "KDSoapServerGlobal.h"
#include <QtCore/QObject>
class KDSoapAuthentication;
class KDSoapServerSocket;

/**
 * Additional interface for handling authentication.
 *
 * In addition to deriving from KDSoapServerObjectInterface, you can derive from
 * KDSoapServerAuthInterface in order to handle HTTP authentication.
 *
 * Use Q_INTERFACES(KDSoapServerAuthInterface) in your derived class (under Q_OBJECT)
 * so that Qt can discover the additional inheritance.
 *
 * \since 1.3
 */
class KDSOAPSERVER_EXPORT KDSoapServerAuthInterface
{
public:
    /**
     * Constructor
     */
    KDSoapServerAuthInterface();

    /**
     * Destructor
     */
    virtual ~KDSoapServerAuthInterface();

    /**
     * Return true if the authentication details are valid.
     */
    virtual bool validateAuthentication(const KDSoapAuthentication &auth, const QString &path);

private:
    friend class KDSoapServerSocket;
    bool handleHttpAuth(const QByteArray &authValue, const QString &path);
    class Private;
    Private *const d;
};

QT_BEGIN_NAMESPACE
Q_DECLARE_INTERFACE(KDSoapServerAuthInterface, "com.kdab.KDSoap.ServerAuthInterface/1.0")
QT_END_NAMESPACE

#endif /* KDSOAPSERVERAUTHINTERFACE_H */
