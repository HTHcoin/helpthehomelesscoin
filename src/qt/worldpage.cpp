#include "worldpage.h"
#include "ui_worldpage.h"

#include <QUrl>
#include <QDesktopServices>
#include <QProcess>






WorldPage::WorldPage(QWidget *parent) :   QWidget(parent),
                                                        ui(new Ui::WorldPage)
{
    ui->setupUi(this);   
								
    ui->pushButton_Website->setStatusTip(tr("Visit Help The Homeless Worldwide A NJ Nonprofit Corporation"));
    ui->pushButton_Website_1->setStatusTip(tr("Visit Help The Homeless Coin"));
    ui->pushButton_Website_2->setStatusTip(tr("Trade Help The Homeless Coin Today!!!"));
   /* ui->pushButton_Amazon->setStatusTip(tr("Visit Amazon Smile for Help The Homeless Worldwide"));   */
    ui->pushButton_Website_4->setStatusTip(tr("Visit HTH Partners and Donate Today!!!!!!!!"));
    ui->pushButton_Website_5->setStatusTip(tr("Check out the latest HTH News!!!"));  
    ui->pushButton_Website_6->setStatusTip(tr("HTHB is trading now!! More HTH Tokens to launch soon!!"));
    ui->pushButton_Website_7->setStatusTip(tr("Visit HTH Chapters to see wherer you can make a difference!!!!"));	
   /* ui->pushButton_Website_8->setStatusTip(tr("Visit Help The Homeless Coin Mining Pools")); */
    ui->pushButton_Website_9->setStatusTip(tr("Trade Help The Homeless on Binance Smart Chain Today!!!"));
    ui->pushButton_Website_10->setStatusTip(tr("Visit Coin Gecko for value of Help The Homeless Coin"));

}



/************** HTH Worldwide Buttons *****************/

void WorldPage::on_pushButton_Website_clicked()
{
 
    QDesktopServices::openUrl(QUrl("https://hth.world/", QUrl::TolerantMode));

}

void WorldPage::on_pushButton_Website_1_clicked()
{
    
    QDesktopServices::openUrl(QUrl("https://hth.world/hth-coin/", QUrl::TolerantMode));	

}


void WorldPage::on_pushButton_Website_2_clicked()
{

    QDesktopServices::openUrl(QUrl("https://hth.world/hth-coin/buy-hthcoin/", QUrl::TolerantMode));

}


/*void WorldPage::on_pushButton_Amazon_clicked()
{

    QDesktopServices::openUrl(QUrl("https://smile.amazon.com/ch/83-1698753/", QUrl::TolerantMode));


}*/


void WorldPage::on_pushButton_Website_4_clicked()
{

   QDesktopServices::openUrl(QUrl("https://hth.world/partners/", QUrl::TolerantMode));


}

void WorldPage::on_pushButton_Website_5_clicked()
{

    QDesktopServices::openUrl(QUrl("https://hth.world/category/hth-coin/", QUrl::TolerantMode));

}

void WorldPage::on_pushButton_Website_6_clicked()
{

   QDesktopServices::openUrl(QUrl("https://hth.world/homeless-tokens/", QUrl::TolerantMode));

}

void WorldPage::on_pushButton_Website_7_clicked()
{

  QDesktopServices::openUrl(QUrl("https://hth.world/nonprofit/hth-chapters/", QUrl::TolerantMode));


}


/*void WorldPage::on_pushButton_Website_8_clicked()
{

    QDesktopServices::openUrl(QUrl("https://miningpoolstats.stream/hthcoin", QUrl::TolerantMode));

}*/

void WorldPage::on_pushButton_Website_9_clicked()
{

    QDesktopServices::openUrl(QUrl("https://hth.world/hthb-homeless-binance-token/trade-hthb-homeless-binance-token/", QUrl::TolerantMode));


}

void WorldPage::on_pushButton_Website_10_clicked()
{

   QDesktopServices::openUrl(QUrl("https://www.coingecko.com/en/coins/help-the-homeless-coin/", QUrl::TolerantMode));

}

	/************** HTH Worldwide Buttons ******************/
 


WorldPage::~WorldPage()
{
    delete ui;
}
