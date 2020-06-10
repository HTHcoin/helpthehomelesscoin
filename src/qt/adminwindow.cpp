#include "adminwindow.h"
#include "ui_adminwindow.h"
#include "statistics.h"
#include "fileman.h"
AdminWindow::AdminWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::AdminWindow)
{
    ui->setupUi(this);
    QWidget::setWindowIcon(QIcon(":/icons/" + theme + "/chat""));
    this->showMaximized();
    this->setWindowTitle("Social Network");

}

AdminWindow::~AdminWindow()
{
    delete ui;
}


void AdminWindow:: makePlot(QVector<Statistics> *plotData)
{
    ui->customPlot->clearGraphs();
    ui->customPlot->legend->setVisible(true);
    ui->customPlot->legend->setFont(QFont("Helvetica", 9));
    QPen pen;
    QStringList lineNames;
    lineNames << "Friends Number" << "Likes Number" << "Posts Number";
    // add graphs with different line styles:
    for (int i = 0; i < 3; ++i)
    {
      ui->customPlot->addGraph();
      if(i == 0)
          pen.setColor(QColor(0x02, 0xa0, 0xc3));
      else if(i == 1)
          pen.setColor(QColor(0xef, 0xef, 0x00));
      else
          pen.setColor(QColor(0xef, 0x2b, 0x90));
      ui->customPlot->graph()->setPen(pen);
      ui->customPlot->graph()->setName(lineNames.at(i));
      ui->customPlot->graph()->setLineStyle((QCPGraph::lsLine));
      ui->customPlot->graph()->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 5));
      // generate data:
      QVector<double> x((*plotData).size()), y(1000);
      for (int j = 0; j < (*plotData).size(); ++j)
      {
        x[j] = (*plotData)[j].getUserID();
        if(i == 0)
            y[j] = (*plotData)[j].getFriendsNumber();
        else if(i == 1)
            y[j] = (*plotData)[j].getLikesNumber();
        else
            y[j] = (*plotData)[j].getPostsNumber();
      }
      ui->customPlot->graph()->setData(x, y);
      ui->customPlot->graph()->rescaleAxes(true);
    }
    // zoom out a bit:
    ui->customPlot->yAxis->scaleRange(0, plotData->size()+1);
    ui->customPlot->xAxis->scaleRange(0, 1000);
    // set blank axis lines:
    ui->customPlot->xAxis->setTicks(true);
    ui->customPlot->yAxis->setTicks(true);
    ui->customPlot->xAxis->setTickLabels(true);
    ui->customPlot->yAxis->setTickLabels(true);
    ui->customPlot->xAxis2->setVisible(true);
    ui->customPlot->yAxis2->setVisible(true);
    ui->customPlot->xAxis2->setTicks(false);
    ui->customPlot->yAxis2->setTicks(false);
    // make top right axes clones of bottom left axes:
//    ui->customPlot->axisRect()->setupFullAxesBox();
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables);
    ui->customPlot->replot();


}
void AdminWindow::on_showStatistics_clicked()
{
    QVector<Statistics> *testingData = new QVector<Statistics>;
    fileman network;
    QList<QString> emails = network.readEmails();
    testingData->resize(emails.size());
    for(int i = 0; i < emails.size(); i++)
    {
        (*testingData)[i].setUserID(i);
        (*testingData)[i].setFriendsNumber(network.getFriends(emails[i]).size() / 3);
        int likesNumber,postsNumber;
        network.getActivity(emails[i],postsNumber,likesNumber);
        (*testingData)[i].setPostsNumber(network.getPosts(emails[i])->size());
        (*testingData)[i].setLikesNumber(likesNumber);
    }
    makePlot(testingData);
}
