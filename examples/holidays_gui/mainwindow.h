#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include "KDSoapClientInterface.h"
#include "KDSoapMessage.h"
#include "wsdl_holidays.h"

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
