#include "../qt/Server/include/socket_server.h"

SocketServer::SocketServer()
{
    connect(&tcp_serv_socket, &QTcpServer::newConnection, this, &SocketServer::on_ClientConnected);

    command_handler = {
        {{"/time"}, time_command_handler}
    };
}

SocketServer::~SocketServer()
{
    stop();
}

void SocketServer::setPort(const quint16 &port)
{
    serv_port = port;
}

void SocketServer::start()
{
    if (tcp_serv_socket.listen(QHostAddress::AnyIPv4, serv_port)) {
        Q_EMIT OutInfo(QString("<-- Server running on port: %1 -->").arg(serv_port));
    }
    else {
        Q_EMIT Error(QString("<-- Could not start the server: (err=%1) -->").arg(tcp_serv_socket.errorString()));
    }
}

void SocketServer::stop()
{
    for (QTcpSocket *connected_client : connectd_clients_list) {
        connected_client->close();
    }

    tcp_serv_socket.close();

    connectd_clients_list.clear();
    ServerDataBase::clear();

    Q_EMIT OutInfo(QString("<-- Server stopped! -->"));
}

void SocketServer::on_ClientConnected()
{
    QTcpSocket *new_client = tcp_serv_socket.nextPendingConnection();

    connect(new_client, &QTcpSocket::stateChanged, this, &SocketServer::on_ClientStateChanged);
    connect(new_client, &QTcpSocket::readyRead, this, &SocketServer::on_ReadyRead);

    QString new_client_connected_msg = QString("<-- New client connected: (addr=%1) -->").arg(new_client->peerAddress().toString());
    
    Q_EMIT OutInfo(new_client_connected_msg);
    
    new_client->write("<---- [ >> Welcome to the Socket-Chat! << ] ---->");

    for (QTcpSocket *connected_client : connectd_clients_list) {
        connected_client->write(new_client_connected_msg.toUtf8());
    }

    connectd_clients_list.append(new_client);
}

void SocketServer::on_ClientStateChanged(QAbstractSocket::SocketState state)
{
    QTcpSocket *client = static_cast<QTcpSocket*>(sender());
    
    if (state == QAbstractSocket::UnconnectedState) {
        QString client_disconnected_msg = QString("<-- Client disconnected: (addr=%1) -->").arg(client->peerAddress().toString());

        if (not ServerDataBase::isEmpty()) {
            ServerDataBase::deleteLastClient();
        }

        connectd_clients_list.removeOne(client);
        client->close();
        Q_EMIT OutInfo(client_disconnected_msg);

        for (QTcpSocket *connected_client : connectd_clients_list) {
            connected_client->write(client_disconnected_msg.toUtf8());
        }
    }
}

void SocketServer::on_ReadyRead()
{
    QTcpSocket *client = static_cast<QTcpSocket*>(sender());

    quint16 next_block_size = 0;
    QByteArray recived_data;

    QString user_name;

    QDataStream recive_stream(client);
    recive_stream.setVersion(QDataStream::Qt_5_7_1);

    forever {
        if (!next_block_size) {
            if (client->bytesAvailable() < sizeof(quint16)) {
                break;
            }
        }
        recive_stream >> next_block_size;

        if (client->bytesAvailable() < next_block_size) {
            break;
        }

        recive_stream >> user_name >> recived_data;
        next_block_size = 0;
    }

    if (not ServerDataBase::isExists(user_name)) {
        ServerDataBase::addClient(user_name);

        Q_EMIT OutInfo(QString("<-- Recived bytes: %1, from: (addr=%2) -->").arg(recived_data.size()).arg(client->peerAddress().toString()));

        for (QTcpSocket *connected_client : connectd_clients_list) {
            if (command_handler.contains(recived_data)) {
                command_handler[recived_data](QString(user_name + ": " + recived_data).toUtf8(), connected_client, this);
            }
            else { // default message recived
                connected_client->write(QString(user_name + ": " + recived_data).toUtf8());
            }
            Q_EMIT OutInfo(QString("<-- Sending bytes: %1, to: (addr=%2) -->").arg(recived_data.size()).arg(connected_client->peerAddress().toString()));
        }
    }
    else {
        client->write(QString("<-- Client with nickanme '%1' already exists! -->").arg(user_name).toUtf8());
        client->close();
    }
}

void SocketServer::time_command_handler(const QByteArray &data, QTcpSocket *client, SocketServer *server_socket)
{
    client->write(data + "\n");
    client->write("Current date and time: " + QDateTime::currentDateTime().toString("dd-MM-yyyy hh:mm:ss").toUtf8());
    Q_EMIT server_socket->OutInfo(QString("<-- Sending bytes: %1, to: (addr=%2) -->").arg(data.size()).arg(client->peerAddress().toString()));
}
