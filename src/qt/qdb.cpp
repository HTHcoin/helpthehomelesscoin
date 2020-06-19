#include "qdb.h"

QDB::QDB()
{

}

QDB::~QDB()
{
    Disconnect();
}

bool QDB::Connect(const QString dbname)
{

    if (!this->db.isOpen())
    {
        this->db = QSqlDatabase::addDatabase("QSQLITE");
        this->db.setDatabaseName(dbname);

        if (!this->db.open())
        {
          return false;
        }
        else
        {
          return true;
        }
    }
    else
    {
        return false;
    }
}

bool QDB::Disconnect()
{
    if (this->db.isOpen())
    {
        this->db.close();
        return true;
    }
    else
    {
        return false;
    }
}

QSqlQuery Query(QString q)
{
    QSqlQuery query(q);
    return query;
}
