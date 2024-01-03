/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>

namespace BLZService {
class BLZServiceSOAP11Binding;
}
class TNS__GetBankResponseType;

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QProgressBar;
class QMovie;
class QTableWidget;
QT_END_NAMESPACE

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);

private slots:
    void syncCalls();
    void asyncCalls();
    void done(int index, const TNS__GetBankResponseType &response);
    void createJob(int index);

private:
    void setBankName(int row, const QString &text);
    void clearResults();

    QPushButton *mBtnAsync;
    QPushButton *mBtnSync;
    QTableWidget *mTableWidget;
    QLabel *mLblAnim;
    QMovie *mMovAnim;
    QStringList mBankCodes;

    BLZService::BLZServiceSOAP11Binding *mService;
};

#endif
