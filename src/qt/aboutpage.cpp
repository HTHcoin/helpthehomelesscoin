#include "bitcoingui.h"
#include "aboutpage.h"
#include "ui_aboutpage.h"

AboutPage::AboutPage(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutPage)
{
    ui->setupUi(this);
}

void AboutPage::on_visitTorPage_clicked()
{
    QDesktopServices::openUrl(QUrl("https://www.torproject.org"));
}

void AboutPage::on_visitCreatorsPage_pressed()
{
    QDesktopServices::openUrl(QUrl("https://izmukx.github.io"));
}

AboutPage::~AboutPage()
{
    delete ui;
}
