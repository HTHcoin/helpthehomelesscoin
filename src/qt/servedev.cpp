#include "servedev.h"
#include <QTcpSocket>

servedev::servedev(ChatWindowPage* pHelloServer,QObject *parent) : QTcpServer(parent)
{
    m_pHelloWindow=pHelloServer;
}

void servedev::incomingConnection(int socketfd)
{
    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketfd);
    clients.insert(client);

    m_pHelloWindow->addMessage("New client from: "+client->peerAddress().toString());

    connect(client, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(client, SIGNAL(disconnected()), this, SLOT(disconnected()));
}

void servedev::readyRead()
{
    QTcpSocket *client = (QTcpSocket*)sender();
    while(client->canReadLine())
    {
        QString line = QString::fromUtf8(client->readLine()).trimmed();
        //qDebug() << "Read line:" << line;
        m_pHelloWindow->addMessage(line);

    }
}

void servedev::disconnected()
{
    QTcpSocket *client = (QTcpSocket*)sender();
    qDebug() << "Client disconnected:" << client->peerAddress().toString();

    clients.remove(client);


}
