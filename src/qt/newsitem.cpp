// Copyright (c) 2018-2020 The REEX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "newsitem.h"
#include "ui_newsitem.h"

#include <QDebug>

/*#include "newsitem.moc" */

NewsItem::NewsItem(const PlatformStyle *platformStyle, QWidget *parent, QDateTime pubDate,QString url,QString title,QString author,QString description) :
    QWidget(parent),
    pubDate(pubDate),
    ui(new Ui::NewsItem),
    url(url),
    title(title),
    author(author),
    description(description)
{
    ui->setupUi(this);

    ui->newsTitle->setText(QString("<a href=\"") + url + QString("\">") + title + QString("</a>"));
    ui->newsAuthor->setText(QString("by ") + author + QString(" | ") + pubDate.date().toString(QString("dd MMM yyyy")));
}

NewsItem::~NewsItem()
{
    delete ui;
}
