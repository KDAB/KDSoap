#include "mainwindow.h"

#include <QPushButton>
#include <QLabel>
#include <QMovie>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow( QWidget *parent ) : QWidget( parent )
{
   
  mBtnSync = new QPushButton(tr("Sync Call"), this);
  mBtnAsync = new QPushButton(tr("Async Call"), this);
  mLblResult = new QLabel(tr("Result: "), this);
  mLblAnim = new QLabel(this);
  
  mMovAnim = new QMovie(QString::fromLatin1(":/animations/anim.mng"));
  
  QVBoxLayout *centralLayout = new QVBoxLayout(this);
  QHBoxLayout *btnsLayout = new QHBoxLayout();
  QHBoxLayout *lblsLayout = new QHBoxLayout();
  
  btnsLayout->addWidget( mBtnSync  );
  btnsLayout->addWidget( mBtnAsync );
  
  lblsLayout->addWidget( mLblAnim  );
  lblsLayout->addWidget( mLblResult );
  
  centralLayout->addLayout( btnsLayout );
  centralLayout->addLayout( lblsLayout );
  
  mLblAnim->setMovie(mMovAnim);
  mMovAnim->start();
  
  connect(mBtnSync, SIGNAL(clicked()), this, SLOT(syncCall()));
  connect(mBtnAsync, SIGNAL(clicked()), this, SLOT(asyncCall()));
  
  mHolidayDates = new USHolidayDates();
  connect(mHolidayDates, SIGNAL(getValentinesDayDone(const TNS__GetValentinesDayResponse&)), 
          this,          SLOT(  done    (const TNS__GetValentinesDayResponse&)));
  
  connect(mHolidayDates, SIGNAL(getValentinesDayError(const KDSoapMessage&)),
          this,          SLOT  (doneError(const KDSoapMessage&)));
  
  mYear = 1960;
  mParameters.setYear(mYear++);                 
}

void MainWindow::done(const TNS__GetValentinesDayResponse& response)
{
    mLblResult->setText(tr("Valentine's day: %1").arg(response.getValentinesDayResult().toString()));
}

void MainWindow::doneError(const KDSoapMessage& error)
{
    QMessageBox::warning(this, tr("Error retrieving valentines day"), error.faultAsString());
}
  
void MainWindow::syncCall()
{
    TNS__GetValentinesDayResponse response = mHolidayDates->getValentinesDay(mParameters);
    mLblResult->setText( tr("Valentine's day: %1").arg(response.getValentinesDayResult().toString()));
    mParameters.setYear( mYear++ );
}

void MainWindow::asyncCall()
{
  mHolidayDates->asyncGetValentinesDay(mParameters);
  mParameters.setYear( mYear++ );
}

