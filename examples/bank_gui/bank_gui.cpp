/****************************************************************************
**
** This file is part of the KD Soap project.
**
** SPDX-FileCopyrightText: 2019 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
**
** SPDX-License-Identifier: MIT
**
****************************************************************************/

#include "mainwindow.h"
#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow *m = new MainWindow();
    m->show();

    return app.exec();
}
