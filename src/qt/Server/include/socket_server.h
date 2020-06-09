#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostInfo>
#include <QDataStream>
#include <QMessageBox>
#include <QDateTime>

#include "server_data_base.h"

class SocketServer : public QObject
{
    Q_OBJECT

public:
    SocketServer();
    void setPort(const quint16 &port);
    ~SocketServer();

public Q_SLOTS:
    void start();
    void stop();

private Q_SLOTS:
    void on_ClientConnected();
    void on_ClientStateChanged(QAbstractSocket::SocketState state);
    void on_ReadyRead();

Q_SIGNALS:
    void addToDataBase(QStringView username);
    void OutInfo(const QString &message);
    void Error(const QString &error_message);

private:
    quint16 serv_port;
    QTcpServer tcp_serv_socket;
    QList<QTcpSocket*> connectd_clients_list;

    QMap<QString, void (*)(const QByteArray&, QTcpSocket*, SocketServer*)> command_handler;
    static void time_command_handler(const QByteArray &data, QTcpSocket *client, SocketServer *server_socket);
};

#endif // SERVER_H
