#include "newsitem.h"

#include <QtCore>
#include <QtNetwork>
#include <QTimer>


#define NEWS_URL "https://reecore.org/category/news/feed"


ui->listNews->setSortingEnabled(true);


 ui->labelNewsStatus->setText("(" + tr("out of sync") + ")");

    connect(&manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(newsFinished(QNetworkReply*)));

    SetLinks();



timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(updateNewsList()));
    timer->setInterval(10 * 1000); // after 10 seconds on the 1st cycle
    timer->setSingleShot(true);
    timer->start();




void NewsPage::updateNewsList()
{
    ui->labelNewsStatus->setVisible(true);

    xml.clear();

    QUrl url(NEWS_URL);
    newsGet(url);
}

void NewsPage::newsGet(const QUrl &url)
{
    QNetworkRequest request(url);

    if (currentReply) {
        currentReply->disconnect(this);
        currentReply->deleteLater();
    }

    currentReply = manager.get(request);

    connect(currentReply, SIGNAL(readyRead()), this, SLOT(newsReadyRead()));
    connect(currentReply, SIGNAL(metaDataChanged()), this, SLOT(newsMetaDataChanged()));
    connect(currentReply, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(newsError(QNetworkReply::NetworkError)));
}

void NewsPage::newsMetaDataChanged()
{
    QUrl redirectionTarget = currentReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectionTarget.isValid()) {
        newsGet(redirectionTarget);
    }
}

void NewsPage::newsReadyRead()
{
    int statusCode = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (statusCode >= 200 && statusCode < 300) {
        QByteArray data = currentReply->readAll();
        xml.addData(data);
    }
}

void NewsPage::newsFinished(QNetworkReply *reply)
{
    Q_UNUSED(reply);

    parseXml();
    ui->labelNewsStatus->setVisible(false);

    // Timer Activation for the news refresh
    timer->setInterval(5 * 60 * 1000); // every 5 minutes
    timer->start();
}

void NewsPage::parseXml()
{
    QString currentTag;
    QString linkString;
    QString titleString;
    QString pubDateString;
    QString authorString;
    QString descriptionString;

    bool insideItem = false;

    for(int i = 0; i < ui->listNews->count(); ++i)
    {
        delete ui->listNews->takeItem(i);
    }

    try {
        while (!xml.atEnd()) {
            xml.readNext();
            if (xml.isStartElement()) {
                currentTag = xml.name().toString();

                if (xml.name() == "item")
                {
                    insideItem = true;
                    titleString.clear();
                    pubDateString.clear();
                    authorString.clear();
                    descriptionString.clear();
                }
            } else if (xml.isEndElement()) {
                if (xml.name() == "item") {
                    if( !linkString.isEmpty() && !linkString.isNull()
                     && !titleString.isEmpty() && !titleString.isNull()
                     && !authorString.isEmpty() && !authorString.isNull()
                     && !pubDateString.isEmpty() && !pubDateString.isNull())
                    {
                        bool found = false;

                        QDateTime qdt = QDateTime::fromString(pubDateString,Qt::RFC2822Date);

                        for(int i = 0; i < ui->listNews->count(); ++i)
                        {
                            NewsItem * item = (NewsItem *)(ui->listNews->itemWidget(ui->listNews->item(i)));
                            if( item->pubDate == qdt )
                            {
                                found = true;
                                break;
                            }
                        }

                        if( !found )
                        {
                            NewsWidgetItem *widgetItem = new NewsWidgetItem(ui->listNews);
                            widgetItem->setData(Qt::UserRole,qdt);

                            ui->listNews->addItem(widgetItem);

                            NewsItem *newsItem = new NewsItem(this,qdt,linkString,titleString,authorString,descriptionString);

                            widgetItem->setSizeHint( newsItem->sizeHint() );

                            ui->listNews->setItemWidget( widgetItem, newsItem );
                        }
                    }

                    titleString.clear();
                    linkString.clear();
                    pubDateString.clear();
                    authorString.clear();
                    descriptionString.clear();

                    insideItem = false;
                }
            } else if (xml.isCharacters() && !xml.isWhitespace()) {
                if (insideItem) {
                    if (currentTag == "title")
                        titleString += xml.text().toString();
                    else if (currentTag == "link")
                        linkString += xml.text().toString();
                    else if (currentTag == "pubDate")
                        pubDateString += xml.text().toString();
                    else if (currentTag == "creator")
                        authorString += xml.text().toString();
                    else if (currentTag == "description")
                        descriptionString += xml.text().toString();
                }
            }
        }

        if (xml.error() && xml.error() != QXmlStreamReader::PrematureEndOfDocumentError) {
            qWarning() << "XML ERROR:" << xml.lineNumber() << ": " << xml.errorString();
        }
    }

    catch(std::exception &e)
    {
        qWarning("std:exception %s",e.what());
    }

    catch(...)
    {
        qWarning("generic exception");
    }
}

void NewsPage::newsError(QNetworkReply::NetworkError)
{
    qWarning("error retrieving RSS feed");

    currentReply->disconnect(this);
    currentReply->deleteLater();
    currentReply = 0;
}
