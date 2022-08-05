/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2019-2022 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#include "mainwindow.h"
#include "wsdl_BLZService.h"

#include <QPushButton>
#include <QLabel>
#include <QHeaderView>
#include <QMovie>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QProgressBar>
#include <QDebug>
#include <QMessageBox>
#include <QTableWidget>

enum Columns {
    Code,
    Name
};

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("KDSoap test program"));
    resize(600, 600);
    mBtnSync = new QPushButton(tr("Sync Calls"), this);
    mBtnAsync = new QPushButton(tr("Async Calls"), this);

    // found on https://www.thebankcodes.com/blz/bybankname.php
    mBankCodes = QStringList { QStringLiteral("10020000"), QStringLiteral("20130600"), QStringLiteral("10090000"), QStringLiteral("55060611"),
                               QStringLiteral("64250040"), QStringLiteral("50310400"), QStringLiteral("50030000"), QStringLiteral("76069601"),
                               QStringLiteral("43051040"), QStringLiteral("71162355") };

    mTableWidget = new QTableWidget(this);
    mTableWidget->setRowCount(mBankCodes.count());
    mTableWidget->setColumnCount(2);
    mTableWidget->setHorizontalHeaderLabels({ tr("Bank code"), tr("Bank name") });
    mTableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    clearResults();

    mLblAnim = new QLabel(this);
    auto animSizePolicy = mLblAnim->sizePolicy();
    animSizePolicy.setRetainSizeWhenHidden(true);
    mLblAnim->setSizePolicy(animSizePolicy);

    mMovAnim = new QMovie(QString::fromLatin1(":/animations/spinner.gif"), {}, this);

    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    QHBoxLayout *btnsLayout = new QHBoxLayout();
    QHBoxLayout *progressLayout = new QHBoxLayout();
    QHBoxLayout *lblsLayout = new QHBoxLayout();

    btnsLayout->addWidget(mBtnSync);
    btnsLayout->addWidget(mBtnAsync);

    lblsLayout->addWidget(mTableWidget);

    progressLayout->addWidget(mLblAnim);

    centralLayout->addLayout(btnsLayout);
    centralLayout->addLayout(progressLayout);
    centralLayout->addLayout(lblsLayout);

    mLblAnim->setMovie(mMovAnim);
    mLblAnim->setFixedSize(66, 66);

    connect(mBtnSync, &QPushButton::clicked, this, &MainWindow::syncCalls);
    connect(mBtnAsync, &QPushButton::clicked, this, &MainWindow::asyncCalls);

    mService = new BLZService::BLZServiceSOAP11Binding(this);
}

void MainWindow::clearResults()
{
    std::random_shuffle(mBankCodes.begin(), mBankCodes.end());
    for (int row = 0; row < mBankCodes.count(); ++row) {
        auto *item = new QTableWidgetItem(mBankCodes.at(row));
        mTableWidget->setItem(row, Columns::Code, item);

        setBankName(row, QString());
    }
}

void MainWindow::syncCalls()
{
    clearResults();
    mLblAnim->show();
    mMovAnim->start();

    TNS__GetBankType parameters;
    for (int index = 0; index < mBankCodes.count(); ++index) {
        parameters.setBlz(mBankCodes[index]);
        auto response = mService->getBank(parameters);
        if (!mService->lastError().isEmpty()) {
            setBankName(index, tr("Error making the SOAP call: %1").arg(mService->lastError()));
        } else {
            setBankName(index, response.details().bezeichnung());
        }
    }

    mMovAnim->stop();
    mLblAnim->hide();
}

void MainWindow::asyncCalls()
{
    clearResults();
    mLblAnim->show();
    mMovAnim->start();
    createJob(0);
}

void MainWindow::createJob(int index)
{
    TNS__GetBankType parameters;
    parameters.setBlz(mBankCodes[index]);
    auto job = new BLZService::BLZServiceSOAP11BindingJobs::GetBankJob(mService, this);
    job->setParameters(parameters);
    job->start();
    connect(job, &KDSoapJob::finished, this, [=]() {
        if (job->isFault()) {
            setBankName(index, tr("Error making the SOAP call: %1").arg(job->reply().faultAsString()));
        } else {
            done(index, job->resultParameters());
        }
    });
}

void MainWindow::setBankName(int row, const QString &text)
{
    auto *item = new QTableWidgetItem(text);
    mTableWidget->setItem(row, Columns::Name, item);
}

void MainWindow::done(int index, const TNS__GetBankResponseType &response)
{
    setBankName(index, response.details().bezeichnung());
    if (index < mBankCodes.count() - 1) {
        createJob(index + 1);
    } else {
        mMovAnim->stop();
        mLblAnim->hide();
    }
}
