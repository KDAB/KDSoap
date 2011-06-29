#ifndef HELLOWORLD_CLIENT_H
#define HELLOWORLD_CLIENT_H

#include "wsdl_helloworld.h"

#include <QMainWindow>

class QLineEdit;
class QTextBrowser;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent=0);

private Q_SLOTS:
    void sayHello();
    void sayHelloDone(const QString& reply);
    void sayHelloError(const KDSoapMessage& fault);

private:
    Hello_Service m_service;
    QLineEdit* m_input;
    QTextBrowser* m_browser;
};

#endif // HELLOWORLD_CLIENT_H
