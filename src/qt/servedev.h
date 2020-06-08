#ifndef SERVEDEV_H
#define SERVEDEV_H
#include<QTcpServer>
#include"chatwindowpage.h"
class ChatWindowPage;
class servedev : public QTcpServer
{
public:

    Q_OBJECT

            public:
                servedev(ChatWindowPage* pHelloServer,QObject *parent=0);
                ChatWindowPage* m_pHelloWindow;

            private Q_SLOTS:
                void readyRead();
                void disconnected();

            protected:
                void incomingConnection(int socketfd);

            private:
                QSet<QTcpSocket*> clients;

};

#endif // SERVEDEV_H
