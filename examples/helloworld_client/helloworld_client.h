/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2011-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef HELLOWORLD_CLIENT_H
#define HELLOWORLD_CLIENT_H

#include "wsdl_helloworld.h"

#include <QWidget>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QTextBrowser;
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private Q_SLOTS:
    void sayHello();
    void sayHelloDone(const QString &reply);
    void sayHelloError(const KDSoapMessage &fault);

private:
    Hello_Service m_service;
    QLineEdit *m_input;
    QTextBrowser *m_browser;
};

#endif // HELLOWORLD_CLIENT_H
