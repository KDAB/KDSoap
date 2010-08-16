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
  void getValentinesDay(const TNS__GetValentinesDayResponse& r);
  void getValentinesDayError(const KDSoapMessage& error);
  
private:
  QPushButton *mBtnAsync;
  QPushButton *mBtnSync;
  QLabel      *mLblResult;
  QLabel      *mLblAnim;
  QMovie      *mMovAnim;
  
  USHolidayDates *mHolidayDates;
  TNS__GetValentinesDay mParameters;
    
  //message.addArgument(QLatin1String("year"), year);

};

#endif