#ifndef SOCKETCLIENT_H
#define SOCKETCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QDateTime>

class SocketClient : public QObject
{
    Q_OBJECT
private:
    quint16 remote_port;
    QString remote_address;
    QTcpSocket tcp_sock;

public:
    explicit SocketClient(QObject *parent = nullptr);
    void setPort(quint16 port);
    void setAddress(const QString &address);
    void start();

private slots:
    void on_Connected();
    void on_Disconnected();
    void on_ReadyRead();
    void DisplayError(QAbstractSocket::SocketError error_code);

public slots:
    void SendToServer(const QString &nickname, const QString &message);
    void stop();

signals:
    void outInfo(const QString &message);
    void Error(const QString &message);
    void started();
    void connected();
    void disconnected();
};

#endif // SOCKETCLIENT_H
