#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    m_pClientsocket = new QTcpSocket(this);
    ui->setupUi(this);
}

Dialog::~Dialog()
{

    delete ui;
}


void Dialog::on_Connect_clicked()
{
    m_pClientsocket->connectToHost(ui->textEditIP->toPlainText(),quint16(ui->textEditPort->toPlainText().toInt()) );
    connect(m_pClientsocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));
}

void Dialog::on_Send_clicked()
{
    QString message = ui->textEdit_3->toPlainText().trimmed();
    //if chat server is not empty
    if(!message.isEmpty())
    {
        m_pClientsocket->write(QString (message + "\n").toUtf8());

    }
    //chat out the input box
    ui->textEdit_3->clear();

    ui->textEdit_3->setFocus();
}
void Dialog::displayError ( QAbstractSocket::SocketError socketError )
{
    switch (socketError) {
         case QAbstractSocket::RemoteHostClosedError:
             break;
         case QAbstractSocket::HostNotFoundError:
             QMessageBox::information(this, tr("Fortune Client"),
                                      tr("The host was not found. Please check the "
                                         "host name and port settings."));
             break;
         case QAbstractSocket::ConnectionRefusedError:
             QMessageBox::information(this, tr("Fortune Client"),
                                      tr("The connection was refused by the peer. "
                                         "Make sure the fortune server is running, "
                                         "and check that the host name and port "
                                         "settings are correct."));
             break;
         default:
             QMessageBox::information(this, tr("Fortune Client"),
                                      tr("The following error occurred: %1.")
                                      .arg(m_pClientsocket->errorString()));
         }


}
