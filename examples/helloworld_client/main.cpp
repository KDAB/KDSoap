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

#include "helloworld_client.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    m_service.setEndPoint(QLatin1String("http://localhost:8081"));
    m_service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
    connect(&m_service, SIGNAL(sayHelloDone(QString)), this, SLOT(sayHelloDone(QString)));
    connect(&m_service, SIGNAL(sayHelloError(KDSoapMessage)), this, SLOT(sayHelloError(KDSoapMessage)));
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(central);
    m_browser = new QTextBrowser;
    QLabel* label = new QLabel;
    label->setWordWrap(true);
    label->setText(tr("<qt><p>This is a simple client/server demo. Start bin/helloworld_server separately on the commandline.</p>"
                   "<p>Clicking &quot;Send&quot; will make a sayHello() soap call. To trigger an error, leave the input field empty and click &quot;Send&quot;.</p>"));
    layout->addWidget(label);
    layout->addWidget(m_browser);
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget* w1 = new QWidget;
    QHBoxLayout* l1 = new QHBoxLayout(w1);
    l1->setContentsMargins(0, 0, 0, 0);
    m_input = new QLineEdit;
    l1->addWidget(m_input);
    QPushButton* pb1 = new QPushButton(tr("Send"));
    l1->addWidget(pb1);
    connect(m_input, SIGNAL(returnPressed()), this, SLOT(sayHello()));
    connect(pb1, SIGNAL(clicked()), this, SLOT(sayHello()));
    layout->addWidget(w1);
    setCentralWidget(central);
}

void MainWindow::sayHello()
{
    m_service.asyncSayHello(m_input->text().trimmed());
}

void MainWindow::sayHelloDone(const QString &reply)
{
    m_browser->append(tr("Reply from server: <font color=\"darkgreen\">%1</font>").arg(reply));
}

void MainWindow::sayHelloError(const KDSoapMessage &fault)
{
    m_browser->append(tr("Error from server: <font color=\"red\">%1</font>").arg(fault.faultAsString()));
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow mw;
    mw.show();
    return app.exec();
}
