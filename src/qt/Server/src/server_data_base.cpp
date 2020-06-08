#include "../include/server_data_base.h"

QVector<QString> ServerDataBase::clients {};

ServerDataBase::ServerDataBase(QObject *parent)
    : QObject(parent)
{}

void ServerDataBase::addClient(QStringView name)
{
    clients.append(name.toString());
}

void ServerDataBase::deleteLastClient()
{
    clients.removeLast();
}

void ServerDataBase::deleteClientByName(QStringView name)
{
    clients.removeOne(name.toString());
}

bool ServerDataBase::isExists(QStringView name)
{
    return clients.contains(name.toString());
}

bool ServerDataBase::isEmpty()
{
    return clients.isEmpty();
}

void ServerDataBase::clear()
{
    clients.clear();
}

QString ServerDataBase::getLast()
{
    return clients.last();
}
