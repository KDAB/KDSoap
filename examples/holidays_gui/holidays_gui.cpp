#include <QApplication>
#include "mainwindow.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow *m = new MainWindow();
    m->show();

    return app.exec();
}
