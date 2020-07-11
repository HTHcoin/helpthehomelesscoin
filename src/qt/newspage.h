#include <QWidget>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QBuffer>
#include <QXmlStreamReader>
#include <QUrl>


namespace Ui
{
class NewsPage;
}



QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

/** NewsPage ("home") page widget */
class NewsPage : public QWidget
{
    Q_OBJECT

public:
    explicit NewsPage(QWidget* parent = 0);
    ~NewsPage();

public Q_SLOTS:
    void updateNewsList();
    void newsFinished(QNetworkReply *reply);
    void newsReadyRead();
    void newsMetaDataChanged();
    void newsError(QNetworkReply::NetworkError);

private:
    QTimer* timer;
    Ui::NewsPage* ui;   

    void parseXml();
    void newsGet(const QUrl &url);

    QXmlStreamReader xml;

    QNetworkAccessManager manager;
    QNetworkReply *currentReply;

};

#endif // BITCOIN_QT_OVERVIEWPAGE_H
