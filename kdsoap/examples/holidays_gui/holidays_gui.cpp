#include <QApplication>
#include "mainwindow.h"


#include <QDebug>

// TODO a button for an async call, a button for a sync call,
// and some background operation that will stop working during the sync call
// (or just a mouseover effect?)

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow *m = new MainWindow();
    m->show();

    return app.exec();
}
