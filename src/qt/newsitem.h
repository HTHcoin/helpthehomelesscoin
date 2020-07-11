// Copyright (c) 2018-2020 The REEX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_NEWSITEM_H
#define BITCOIN_QT_NEWSITEM_H

#include <QtCore>
#include <QWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QUrl>

namespace Ui
{
class NewsItem;
}

/** NewsItem widget */
class NewsItem : public QWidget
{
    Q_OBJECT

public:
    explicit NewsItem(QWidget* parent,QDateTime pubDate,QString url,QString title,QString author,QString description);
    ~NewsItem();

    QDateTime pubDate;

private:
    Ui::NewsItem* ui;

    QString url;
    QString title;
    QString author;
    QString description;
};

class NewsWidgetItem : public QListWidgetItem
{
    //Q_OBJECT

public:
    explicit NewsWidgetItem(QListWidget *parent = nullptr, int type = Type) : QListWidgetItem(parent,type)
    {
    }

    ~NewsWidgetItem()
    {
    }

    virtual bool operator<(const QListWidgetItem &other) const
    {
        return data(Qt::UserRole) > other.data(Qt::UserRole);
    }
};

#endif // BITCOIN_QT_NEWSITEM_H
