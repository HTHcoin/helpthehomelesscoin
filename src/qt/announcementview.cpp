// Copyright (c) 2018 The Social Send developers
// Copyright (c) 2019 The Crypto Dezire Cash developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "announcementview.h"
#include "ui_announcementview.h"
#include <QMessageBox>
#include <QModelIndex>

AnnouncementView::AnnouncementView(QWidget *parent) :   QWidget(parent),
                                                        ui(new Ui::AnnouncementView)
{
    ui->setupUi(this);
    ui->verticalLayout_3->setAlignment(Qt::AlignTop);
    lastUpdate = 0;
    ui->mainTitleLable->setText(tr("HTH Worldwide Announcements"));
    ui->pushButton->setText(tr("Refresh"));
    //Load Announcement data
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("https://wp.me/Pauece-7W/")));
}

AnnouncementView::~AnnouncementView()
{
    delete ui;
}

void AnnouncementView::replyFinished(QNetworkReply *reply)
{
    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();

    }
    else
    {

        QString json = reply->readAll();

        QJsonDocument jdoc = QJsonDocument::fromJson(json.toUtf8());
        if(jdoc.isNull()){
            reply->deleteLater();
            return;
        }

        QJsonObject response = jdoc.object();

        QString status = response["status"].toString();
        if(status.isEmpty() || (status != "ok") ){
            reply->deleteLater();
            return;
        }

        QJsonArray jsonANNs = response["posts"].toArray();
        annList.clear();
        annNumber = 0;

        int i = 0;
        Q_FOREACH(QJsonValue obj, jsonANNs){
            if (i > 5) break;
            QJsonObject ann = obj.toObject();
            AnnData a;
            a.title = ann["title"].toString();
            a.info = ann["content"].toString();
            a.imageURL = ann["thumbnail"].toString();
            a.link =ann["url"].toString();
            annList << a;
            i++;
        }

        if(annList.isEmpty()){
            reply->deleteLater();
            return;
        }
        int time = response["time"].toInt();
        if(time > lastUpdate){

            lastUpdate = time;

            QLayoutItem *child;
            while ((child = ui->verticalLayout->takeAt(0)) != 0) {
                delete child->widget();
                delete child;
            }

            QNetworkAccessManager *manager = new QNetworkAccessManager(this);
            connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedImage(QNetworkReply*)));

            manager->get(QNetworkRequest(QUrl(annList[0].imageURL)));
        }else{
             ui->pushButton->setEnabled(true);
        }


    }
    reply->deleteLater();
}

void AnnouncementView::replyFinishedImage(QNetworkReply *reply)
{
    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();

    }
    else
    {


        QByteArray jpegData = reply->readAll();
        QPixmap pixmap;

        pixmap.loadFromData(jpegData);
        if (!pixmap.isNull())
        {
            annList[annNumber].image = pixmap;
            annWidget *ann_item = new annWidget(this);
            annList[annNumber].height = ann_item->getSize().height();
            ann_item->setData(annList[annNumber]);

            ui->verticalLayout->addWidget(ann_item);

            annNumber++;
            if(annNumber <= annList.size() -1){
                QNetworkAccessManager *manager = new QNetworkAccessManager(this);
                connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinishedImage(QNetworkReply*)));

                manager->get(QNetworkRequest(QUrl(annList[annNumber].imageURL)));
            }else{
                ui->pushButton->setEnabled(true);
            }
        }



    }
    reply->deleteLater();
}




void AnnouncementView::on_pushButton_clicked()
{
    annList.clear();
    annNumber=0;


    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("https://wp.me/Pauece-7W/")));
    ui->pushButton->setEnabled(false);
}
