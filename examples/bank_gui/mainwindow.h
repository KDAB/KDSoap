/****************************************************************************
**
** This file is part of the KD Soap library.
**
** SPDX-FileCopyrightText: 2019-2020 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "wsdl_BLZService.h"

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QMovie;
QT_END_NAMESPACE

class MainWindow : public QWidget{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);

private slots:
  void syncCall();
  void asyncCall();
  void done(const TNS__GetBankResponseType& response);
  void doneError(const KDSoapMessage& error);

private:
  void nextBank();

  QPushButton *mBtnAsync;
  QPushButton *mBtnSync;
  QLabel      *mLblResult;
  QLabel      *mLblAnim;
  QMovie      *mMovAnim;

  int mIndex = 0;

  BLZService::BLZServiceSOAP11Binding *mService;
  TNS__GetBankType mParameters;
};

#endif
