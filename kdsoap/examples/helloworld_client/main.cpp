#include "helloworld_client.h"

#include <QApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextBrowser>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    m_service.setEndPoint(QLatin1String("http://localhost:8081"));
    m_service.setSoapVersion(KDSoapClientInterface::SOAP1_2);
    connect(&m_service, SIGNAL(sayHelloDone(QString)), this, SLOT(sayHelloDone(QString)));
    connect(&m_service, SIGNAL(sayHelloError(KDSoapMessage)), this, SLOT(sayHelloError(KDSoapMessage)));
    QWidget* central = new QWidget;
    QVBoxLayout* layout = new QVBoxLayout(central);
    m_browser = new QTextBrowser;
    QLabel* label = new QLabel;
    label->setWordWrap(true);
    label->setText(tr("<qt><p>This is a simple client/server demo. Start bin/helloworld_server separately on the commandline.</p>"
                   "<p>Clicking &quot;Send&quot; will make a sayHello() soap call. To trigger an error, leave the input field empty and click &quot;Send&quot;.</p>"));
    layout->addWidget(label);
    layout->addWidget(m_browser);
    layout->setMargin(0);
    QWidget* w1 = new QWidget;
    QHBoxLayout* l1 = new QHBoxLayout(w1);
    l1->setMargin(0);
    m_input = new QLineEdit;
    l1->addWidget(m_input);
    QPushButton* pb1 = new QPushButton(tr("Send"));
    l1->addWidget(pb1);
    connect(m_input, SIGNAL(returnPressed()), this, SLOT(sayHello()));
    connect(pb1, SIGNAL(clicked()), this, SLOT(sayHello()));
    layout->addWidget(w1);
    setCentralWidget(central);
}

void MainWindow::sayHello()
{
    m_service.asyncSayHello(m_input->text().trimmed());
}

void MainWindow::sayHelloDone(const QString &reply)
{
    m_browser->append(tr("Reply from server: <font color=\"darkgreen\">%1</font>").arg(reply));
}

void MainWindow::sayHelloError(const KDSoapMessage &fault)
{
    m_browser->append(tr("Error from server: <font color=\"red\">%1</font>").arg(fault.faultAsString()));
}

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow mw;
    mw.show();
    return app.exec();
}
