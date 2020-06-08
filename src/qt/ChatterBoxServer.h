#ifndef __ChatterBoxServer_H__
#define __ChatterBoxServer_H__

#include <QStringList>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QSet>

class ChatterBoxServer : public QTcpServer
{
    Q_OBJECT

    public:
        ChatterBoxServer(QObject *parent=0);

    private slots:
        void readyRead();
        void disconnected();
        void sendUserList();

    protected:
        void incomingConnection(int socketfd);

    private:
        QSet<QTcpSocket*> clients;
        QMap<QTcpSocket*,QString> users;
};

#endif
