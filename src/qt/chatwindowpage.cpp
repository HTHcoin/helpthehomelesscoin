#include "chatwindowpage.h"
#include "ui_chatwindowpage.h"
#include"servedev.h"
ChatWindowPage::ChatWindowPage(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ChatWindowPage)
{
    ui->setupUi(this);
    //m_pBoxServer = new servedev(this);

}

ChatWindowPage::~ChatWindowPage()
{
    delete ui;
}

void ChatWindowPage::addMessage(QString Msg)
{

    ui->textEditStatus->setText(Msg);

}

void ChatWindowPage::on_pushButtonStart_clicked()
{
    m_pBoxServer = new servedev(this);

    bool success = m_pBoxServer->listen(QHostAddress::Any, quint16(ui->textEditPort->toPlainText().toInt()));
    if(!success)
    {
        addMessage("Server failed...");

    }
    else
    {
        addMessage("Server Started...");
    }

}
