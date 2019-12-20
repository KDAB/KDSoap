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

  mService = new BLZService::BLZServiceSOAP11Binding(this);

  connect(mService, SIGNAL(getBankDone(TNS__GetBankResponseType)),
          this,     SLOT(done(const TNS__GetBankResponseType&)));

  connect(mService, SIGNAL(getBankError(const KDSoapMessage&)),
          this,          SLOT  (doneError(const KDSoapMessage&)));

  nextBank();
}

void MainWindow::done(const TNS__GetBankResponseType &response)
{
    mLblResult->setText(tr("Bank found: \"%1\"").arg(response.details().bezeichnung()));
}

void MainWindow::doneError(const KDSoapMessage& error)
{
    QMessageBox::warning(this, tr("Error making the SOAP call"), error.faultAsString());
}

void MainWindow::nextBank()
{
    // found on https://www.thebankcodes.com/blz/bybankname.php
    static const char* blzList[] = { "10020000", "20130600", "10090000" };
    static const int numEntries = sizeof(blzList) / sizeof(*blzList);
    mIndex = (mIndex + 1) % numEntries;
    mParameters.setBlz(QString::fromLatin1(blzList[mIndex]));
}

void MainWindow::syncCall()
{
    auto response = mService->getBank(mParameters);
    mLblResult->setText( tr("Bank found: %1").arg(response.details().bezeichnung()));
    nextBank();
}

void MainWindow::asyncCall()
{
    mService->asyncGetBank(mParameters);
    nextBank();
}

