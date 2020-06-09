#include "../include/socket_client.h"

SocketClient::SocketClient(QObject *parent)
    : QObject(parent)
    , remote_port {4455}
    , remote_address {"127.0.0.1"}
{
    connect(&tcp_sock, &QTcpSocket::connected, this, &SocketClient::on_Connected);
    connect(&tcp_sock, &QTcpSocket::disconnected, this, &SocketClient::on_Disconnected);
    connect(&tcp_sock, &QTcpSocket::readyRead, this, &SocketClient::on_ReadyRead);
    connect(&tcp_sock, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &SocketClient::DisplayError);
}

void SocketClient::setPort(quint16 port)
{
    remote_port = port;
}

void SocketClient::setAddress(const QString &address)
{
    remote_address = address;
}

void SocketClient::start()
{
    Q_EMIT started();
    tcp_sock.connectToHost(remote_address, remote_port);
}

void SocketClient::stop()
{
    tcp_sock.disconnectFromHost();
}

void SocketClient::on_Connected()
{
    Q_EMIT connected();
    Q_EMIT outInfo(QString("[%1] You connected with: %2:%3!").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss"))
                 .arg(remote_address).arg(remote_port));
}

void SocketClient::on_Disconnected()
{
    Q_EMIT disconnected();
    Q_EMIT outInfo(QString("[%1] You disconnected from: %2:%3!").arg(QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss"))
                 .arg(remote_address).arg(remote_port));
}

void SocketClient::on_ReadyRead()
{
    Q_EMIT outInfo(tcp_sock.readAll());
}

void SocketClient::DisplayError(QAbstractSocket::SocketError error_code)
{
    switch (error_code) {
    case QAbstractSocket::ConnectionRefusedError:
        Q_EMIT Error(QString("Connection refused with: %1:%2!").arg(remote_address).arg(remote_port));
        break;
    case QAbstractSocket::HostNotFoundError:
        Q_EMIT Error(QString("Remote host not found!"));
        break;
    case QAbstractSocket::RemoteHostClosedError:
        Q_EMIT Error(QString("Remote host is closed!"));
        break;
    }
}

void SocketClient::SendToServer(const QString &nickname, const QString &message)
{
    QByteArray data_block;
    QDataStream send_stream(&data_block, QIODevice::ReadWrite);

    send_stream.setVersion(QDataStream::Qt_5_7_1);

    send_stream << quint16(0) << nickname << message.toUtf8();
    send_stream.device()->seek(0);
    send_stream << quint16(data_block.size() - sizeof(quint16));

    tcp_sock.write(data_block);
}
