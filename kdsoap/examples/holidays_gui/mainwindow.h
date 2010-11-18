/****************************************************************************
** Copyright (C) 2010-2010 Klaralvdalens Datakonsult AB.  All rights reserved.
**
** This file is part of the KD SOAP library.
**
** Licensees holding valid commercial KD Soap licenses may use this file in
** accordance with the KD Soap Commercial License Agreement provided with
** the Software.
**
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 and version 3 as published by the
** Free Software Foundation and appearing in the file LICENSE.GPL included.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** Contact info@kdab.com if any conditions of this licensing are not
** clear to you.
**
**********************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "wsdl_holidays.h"

class QPushButton;
class QLabel;
class QMovie;

class MainWindow : public QWidget{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = 0);
  
private slots:
  void syncCall();
  void asyncCall();
  void done(const TNS__GetValentinesDayResponse& r);
  void doneError(const KDSoapMessage& error);
  
private:
  QPushButton *mBtnAsync;
  QPushButton *mBtnSync;
  QLabel      *mLblResult;
  QLabel      *mLblAnim;
  QMovie      *mMovAnim;
  
  unsigned int mYear;
  
  USHolidayDates *mHolidayDates;
  TNS__GetValentinesDay mParameters;
};

#endif
