/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2011-2021 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#ifndef HELLOWORLD_CLIENT_H
#define HELLOWORLD_CLIENT_H

#include "wsdl_helloworld.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTextBrowser;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=0);

private Q_SLOTS:
    void sayHello();
    void sayHelloDone(const QString& reply);
    void sayHelloError(const KDSoapMessage& fault);

private:
    Hello_Service m_service;
    QLineEdit* m_input;
    QTextBrowser* m_browser;
};

#endif // HELLOWORLD_CLIENT_H
