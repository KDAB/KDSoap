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

#include <QPushButton>
#include <QLabel>
#include <QMovie>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QProgressBar>
#include <QDebug>
#include <QMessageBox>

static int s_numCalls = 10;

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    setWindowTitle(tr("KDSoap test program"));
    resize(600, 200);
    mBtnSync = new QPushButton(tr("Sync Calls"), this);
    mBtnAsync = new QPushButton(tr("Async Calls"), this);
    mLblResult = new QLabel(tr("Result: "), this);
    mLblAnim = new QLabel(this);

    mMovAnim = new QMovie(QString::fromLatin1(":/animations/spinner.gif"), {}, this);
    mProgressBar = new QProgressBar(this);
    mProgressBar->setMaximum(s_numCalls);

    QVBoxLayout *centralLayout = new QVBoxLayout(this);
    QHBoxLayout *btnsLayout = new QHBoxLayout();
    QHBoxLayout *progressLayout = new QHBoxLayout();
    QHBoxLayout *lblsLayout = new QHBoxLayout();

    btnsLayout->addWidget(mBtnSync);
    btnsLayout->addWidget(mBtnAsync);

    lblsLayout->addWidget(mLblResult);

    progressLayout->addWidget(mLblAnim);
    progressLayout->addWidget(mProgressBar);

    centralLayout->addLayout(btnsLayout);
    centralLayout->addLayout(progressLayout);
    centralLayout->addLayout(lblsLayout);

    mLblAnim->setMovie(mMovAnim);
    mLblAnim->setFixedSize(66, 66);

    connect(mBtnSync, &QPushButton::clicked, this, &MainWindow::syncCall);
    connect(mBtnAsync, &QPushButton::clicked, this, &MainWindow::asyncCall);

    mService = new BLZService::BLZServiceSOAP11Binding(this);
}

void MainWindow::nextBank()
{
    // found on https://www.thebankcodes.com/blz/bybankname.php
    static const char *blzList[] = { "10020000", "20130600", "10090000" };
    static const int numEntries = sizeof(blzList) / sizeof(*blzList);
    mIndex = (mIndex + 1) % numEntries;
    mParameters.setBlz(QString::fromLatin1(blzList[mIndex]));
}

void MainWindow::syncCall()
{
    mMovAnim->start();
    mProgressBar->setValue(0);

    for (int i = 0; i < s_numCalls; ++i) {
        nextBank();
        auto response = mService->getBank(mParameters);
        mLblResult->setText(tr("Bank found: %1").arg(response.details().bezeichnung()));
        mProgressBar->setValue(i + 1);
    }

    mMovAnim->stop();
}

void MainWindow::asyncCall()
{
    mMovAnim->start();
    mProgressBar->setValue(0);
    nextJob();
}

void MainWindow::nextJob()
{
    nextBank();
    auto job = new BLZService::BLZServiceSOAP11BindingJobs::GetBankJob(mService, this);
    job->setParameters(mParameters);
    job->start();
    connect(job, &KDSoapJob::finished, this, [=]() {
        if (job->isFault()) {
            QMessageBox::warning(this, tr("Error making the SOAP call"), job->reply().faultAsString());
        } else {
            done(job->resultParameters());
        }
    });
}

void MainWindow::done(const TNS__GetBankResponseType &response)
{
    mLblResult->setText(tr("Bank found: \"%1\"").arg(response.details().bezeichnung()));
    int progress = mProgressBar->value();
    if (progress < s_numCalls) {
        mProgressBar->setValue(++progress);
        nextJob();
    } else {
        mMovAnim->stop();
    }
}
